#!/usr/bin/env bash
# TuxBlox Mac - assemble a portable AppImage and tarball from build/native.
# Copyright (C) 2026 TuxBlox Developers
#
# Usage:
#   packaging/appimage/build-appimage.sh [native-dir] [output-dir]
#
# The native-dir is the directory `make launcher` produces (TuxBloxLauncher and
# libtuxblox). Set APPIMAGETOOL to appimagetool (or its extracted AppRun) to
# produce the AppImage; without it the script writes the portable tarball.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
NATIVE="${1:-$ROOT/build/native}"
OUT="${2:-$ROOT/build/release}"
ROSEMARY="${ROSEMARY:-}"

fail() {
    printf '!! %s\n' "$*" >&2
    exit 1
}

[ -x "$NATIVE/TuxBloxLauncher" ] || fail "$NATIVE/TuxBloxLauncher is missing; run make launcher first"
[ -d "$NATIVE/libtuxblox" ] || fail "$NATIVE/libtuxblox is missing; run make launcher first"
[ -f "$ROOT/build/assets/tuxblox/256x256.png" ] || fail "the 256x256 icon is missing; run make launcher first"
[ -f "$ROOT/packaging/appimage/AppRun" ] || fail "packaging/appimage/AppRun is missing"
[ -f "$ROOT/packaging/appimage/tuxblox-mac.desktop" ] || fail "packaging/appimage/tuxblox-mac.desktop is missing"

VERSION="$("$NATIVE/TuxBloxLauncher" --version)"
[ -n "$VERSION" ] || fail "the launcher did not report a version"

APPDIR="$OUT/TuxBloxMac.AppDir"
APPIMAGE="$OUT/TuxBloxMac-$VERSION-x86_64.AppImage"
TARBALL="$OUT/TuxBloxMac-$VERSION-x86_64.tar.zst"

if [ -z "$ROSEMARY" ]; then
    ROSEMARY="$NATIVE/compat/rosemary"
fi
[ -x "$ROSEMARY" ] || fail "no compatibility layer at $ROSEMARY; run make compat first"

rm -rf "$APPDIR" "$APPIMAGE" "$TARBALL"
mkdir -p \
    "$APPDIR/usr/bin/compat" \
    "$APPDIR/usr/share/tuxblox-mac" \
    "$APPDIR/usr/share/licenses" \
    "$APPDIR/usr/share/applications" \
    "$APPDIR/usr/share/icons/hicolor/256x256/apps"

install -m 755 "$NATIVE/TuxBloxLauncher" "$APPDIR/usr/bin/TuxBloxLauncher"
cp -a "$NATIVE/libtuxblox" "$APPDIR/usr/bin/libtuxblox"
install -m 755 "$ROSEMARY" "$APPDIR/usr/bin/compat/rosemary"
install -m 755 "$ROOT/packaging/appimage/AppRun" "$APPDIR/AppRun"
install -m 644 "$ROOT/packaging/appimage/tuxblox-mac.desktop" "$APPDIR/tuxblox-mac.desktop"
install -m 644 "$ROOT/packaging/appimage/tuxblox-mac.desktop" \
    "$APPDIR/usr/share/applications/tuxblox-mac.desktop"
install -m 644 "$ROOT/build/assets/tuxblox/256x256.png" "$APPDIR/tuxblox.png"
install -m 644 "$ROOT/build/assets/tuxblox/256x256.png" \
    "$APPDIR/usr/share/icons/hicolor/256x256/apps/tuxblox.png"
install -m 644 "$ROOT/LICENSE" "$APPDIR/usr/share/licenses/TuxBlox-LICENSE"
cp -a "$ROOT/third_party_licenses/." "$APPDIR/usr/share/licenses/"
printf '%s\n' "$VERSION" > "$APPDIR/usr/share/tuxblox-mac/VERSION"

TOOL="${APPIMAGETOOL:-}"
if [ -z "$TOOL" ]; then
    TOOL="$(command -v appimagetool 2>/dev/null || true)"
fi
if [ -n "$TOOL" ] && [ -x "$TOOL" ]; then
    printf ':: Building %s\n' "$(basename "$APPIMAGE")"
    ARCH=x86_64 "$TOOL" --no-appstream "$APPDIR" "$APPIMAGE"
else
    printf '!! appimagetool not found; writing the portable tarball only\n' >&2
fi

printf ':: Writing %s\n' "$(basename "$TARBALL")"
tar --zstd -cf "$TARBALL" -C "$OUT" "TuxBloxMac.AppDir"

files=()
if [ -f "$APPIMAGE" ]; then
    files+=("$(basename "$APPIMAGE")")
fi
files+=("$(basename "$TARBALL")")
(
    cd "$OUT"
    sha256sum "${files[@]}" > sha256sums.txt
)

ls -lh "$OUT"/*.AppImage "$OUT"/*.tar.zst "$OUT"/sha256sums.txt 2>/dev/null || true
