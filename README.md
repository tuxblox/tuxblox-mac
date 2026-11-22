# TuxBlox Mac

The complete TuxBlox launcher, with the experimental macOS Player backend and the Windows Studio path inherited from TuxBlox.

This repository holds the launcher (Home, Sessions, Versions, FastFlags, Settings, About, the shared interface, flags, crypto, translations and assets) and `compat/`, the macOS compatibility layer. That layer is a fork of [Rosemary](https://github.com/microlift/rosemary) and runs the official macOS Player natively, without Wine. The Wine-based Windows compatibility layer is **not** in this repository; Studio keeps using the one from an existing TuxBlox install.

## Status

- The complete launcher UI is here: Home, Sessions, Versions, FastFlags, Settings and About.
- The macOS Player backend is experimental and sits behind a local-only flag.
- Studio is inherited unchanged and still uses the TuxBlox compatibility layer.
- Released builds are x86_64. arm64 is a work in progress.

## Download

The latest release includes a ready x86_64 AppImage and a portable tarball:

<https://github.com/tuxblox/tuxblox-mac/releases/latest>

```sh
chmod +x TuxBloxMac-*-x86_64.AppImage
./TuxBloxMac-*-x86_64.AppImage
```

The first run copies the launcher, its interface stack and Rosemary to `~/.local/share/tuxblox-mac`, writes the macOS Player flag, and starts. Later runs reuse the same directory and replace the launcher when the AppImage version changes. Set `TUXBLOX_MAC_DATA` to use another directory.

The AppImage needs FUSE. Where FUSE is unavailable, download the `.tar.zst`, extract it, and run `TuxBloxMac.AppDir/AppRun`; it is the same payload.

The release is Player-only. Studio still needs the TuxBlox compatibility layer; use the drop-in install below if you want Studio.

## What it does

- Downloads the official macOS Player from Roblox's setup CDN.
- Verifies the Apple code signature against the pinned Apple Root CA before installing anything.
- Installs the unmodified bundle under `<install>/runtime` with a local integrity manifest.
- Starts it through Rosemary, with the same launcher, Sessions, settings, Discord Rich Presence and server-region features as TuxBlox.
- Keeps Studio on the Windows compatibility layer, with no changes to its code path.

Nothing patches either client. The only file written inside the macOS bundle is `ClientSettings` for FastFlags.

## Two ways to install

### Player only, without TuxBlox

The macOS Player does not need the Windows compatibility layer. Install the launcher into its own directory (for example `~/.local/share/tuxblox-mac`), add the flag and Rosemary, and run it. Launching Studio reports that the TuxBlox compatibility layer is required; it does not download the Windows client.

### Player and Studio

Studio still needs the Windows compatibility layer. Install the official TuxBlox first, then replace its launcher with the build from this repository:

```sh
install -m 755 build/native/TuxBloxLauncher ~/.tuxblox/TuxBloxLauncher
```

The existing `~/.tuxblox/compat` and `~/.tuxblox/libtuxblox` are left untouched and Studio keeps using them. This repository does not duplicate or modify that layer.

## Requirements

- Linux x86_64.
- For the Player: Rosemary at `<install>/compat/rosemary`.
- For Studio: an existing TuxBlox install (`~/.tuxblox` by default) with its Windows compatibility layer in `compat/` and `libtuxblox/`.
- For the build: `git`, `make`, rootless `podman`, `rsvg-convert` (librsvg) and network access. The build container provides GTK4, libadwaita, libcurl, libarchive, OpenSSL, zlib, CMake and Ninja.

## Building

Clone with the pinned submodules:

```sh
git clone --recurse-submodules https://github.com/tuxblox/tuxblox-mac.git
cd tuxblox-mac
```

If the repository was cloned without them:

```sh
git submodule update --init --recursive
```

Then build the launcher target:

```sh
TUXBLOX_DONT_INCREMENT_BUILD=1 JOBS=2 make launcher
```

`JOBS=2` is a safe default on 8 GB machines; raise it on a larger one. The first build creates the `tuxblox-builder` container image, then builds the shared libraries and the interface stack. It needs several GB of disk and takes a while. Later builds reuse the image and the artifacts.

The result is:

```text
build/native/TuxBloxLauncher
build/native/libtuxblox/
```

`build/native` is the complete launcher directory: the binary and the interface stack it loads.

The macOS compatibility layer is a separate target, built from `compat/` in this tree:

```sh
make compat
```

That writes `build/native/compat/rosemary`. It is a large Rust build and the first one takes a while. The Wine-based Windows compatibility layer Studio uses is not built from this tree.

To assemble the release artifacts:

```sh
APPIMAGETOOL=/path/to/appimagetool make mac-appimage
```

Without `APPIMAGETOOL`, the target still writes the portable `.tar.zst`. It needs the compatibility layer at `build/native/compat/rosemary`, so run `make compat` first or point `ROSEMARY` at a built one.

## Installing the Player only

```sh
DIR="$HOME/.local/share/tuxblox-mac"
mkdir -p "$DIR"
install -m 755 build/native/TuxBloxLauncher "$DIR/TuxBloxLauncher"
cp -a build/native/libtuxblox "$DIR/libtuxblox"
install -m 755 build/native/compat/rosemary "$DIR/compat/rosemary"
```

Create `"$DIR/flags.json"` before the first start:

```json
{
  "FFlagDisableUpdater": true
}
```

Then run:

```sh
"$DIR/TuxBloxLauncher"
```

The first Player launch downloads the official client and verifies its Apple signature. The download is about 160 MB and the installed client takes about 550 MB, so keep roughly 1 GB free.

## Installing beside TuxBlox

Install the official TuxBlox normally, then put this launcher in its directory:

```sh
install -m 755 build/native/TuxBloxLauncher ~/.tuxblox/TuxBloxLauncher
```

Keep the existing `libtuxblox/` and `runtime/` directories. Add the flag to `~/.tuxblox/flags.json`, keeping any entries already there:

```json
{
  "FFlagDisableUpdater": true
}
```

Restart the launcher.

`FFlagDisableUpdater` is recommended while this is a separate build: the official updater or installer can replace this launcher with the upstream one. Do not run the official installer over this launcher.

## Updates

The launcher carries TuxBlox's updater. Until it has its own release channel, keep `FFlagDisableUpdater` set in `flags.json` while testing.

Discord Rich Presence and server-region settings are read when a Player session starts. Restart the Player after changing them.

## Troubleshooting

- **Studio says the compatibility layer is missing.** This is the Player-only layout. Install TuxBlox and use its directory if you want Studio.
- **The Player closes right after opening.** Check that `lms.roblox.com` resolves normally. Some DNS filters return `0.0.0.0` for it; the macOS client cannot finish its login flow without that endpoint. `www.roblox.com` and `clientsettings.roblox.com` being reachable is not enough.
- **The Player does not start.** Confirm that `<install>/compat/rosemary` exists and is executable.

## Licensing

TuxBlox's own code is GPLv3. `compat/` is a fork of Rosemary and is GPL-3.0; it builds to a separate executable and is not linked into the launcher. Its upstream licence is in `third_party_licenses/rosemary/LICENSE`. The Wine-based Windows compatibility layer Studio uses is not part of this repository: in the main TuxBlox repository it stays LGPLv2.1, with `compat/tuxblox/` and `compat/webkitgtk/bundle/` under GPLv3.

The launcher runs the official, unmodified Roblox clients. It is not affiliated with, endorsed by, or associated with Roblox Corporation.
