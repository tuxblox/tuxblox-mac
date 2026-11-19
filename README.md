# TuxBlox Mac

The complete TuxBlox launcher, with the experimental macOS Player backend and the Windows Studio path inherited from TuxBlox.

This repository is the launcher half of TuxBlox: Home, Sessions, Versions, FastFlags, Settings, About, the shared interface, flags, crypto, translations and assets. It deliberately does **not** contain `compat/`, the Wine-based Windows compatibility layer. The macOS Player runs natively through Rosemary; Studio keeps using the compatibility layer from an existing TuxBlox install.

## Status

- The complete launcher UI is here: Home, Sessions, Versions, FastFlags, Settings and About.
- The macOS Player backend is experimental and sits behind a local-only flag.
- Studio is inherited unchanged and still uses the TuxBlox compatibility layer.
- Builds are x86_64 for now. arm64 is not supported yet.

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
- Installs the unmodified bundle under `<install>/mac-client` with a local integrity manifest.
- Starts it through Rosemary, with the same launcher, Sessions, settings, Discord Rich Presence and server-region features as TuxBlox.
- Keeps Studio on the Windows compatibility layer, with no changes to its code path.

Nothing patches either client. The only file written inside the macOS bundle is `ClientSettings` for FastFlags.

## Two ways to install

### Player only, without TuxBlox

The macOS Player does not need `compat/`. Install the launcher into its own directory (for example `~/.local/share/tuxblox-mac`), add the flag and Rosemary, and run it. Launching Studio reports that the TuxBlox compatibility layer is required; it does not download the Windows client.

### Player and Studio

Studio still needs the Windows compatibility layer. Install the official TuxBlox first, then replace its launcher with the build from this repository:

```sh
install -m 755 build/native/TuxBloxLauncher ~/.tuxblox/TuxBloxLauncher
```

The existing `~/.tuxblox/compat` and `~/.tuxblox/libtuxblox` are left untouched and Studio keeps using them. This repository does not duplicate or modify that layer.

## Requirements

- Linux x86_64.
- For the Player: Rosemary at `TUXBLOX_ROSEMARY`, `<install>/rosemary/rosemary`, or `rosemary` on `PATH`.
- For Studio: an existing TuxBlox install (`~/.tuxblox` by default) with `compat/` and `libtuxblox/`.
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

`build/native` is the complete launcher directory: the binary and the interface stack it loads. The Windows compatibility layer is not built from this tree.

To assemble the release artifacts:

```sh
APPIMAGETOOL=/path/to/appimagetool make mac-appimage
```

Without `APPIMAGETOOL`, the target still writes the portable `.tar.zst`. Rosemary is downloaded from the pinned release if it is not already under `build/rosemary`.

## Installing the Player only

```sh
DIR="$HOME/.local/share/tuxblox-mac"
mkdir -p "$DIR"
install -m 755 build/native/TuxBloxLauncher "$DIR/TuxBloxLauncher"
cp -a build/native/libtuxblox "$DIR/libtuxblox"
./shared/tools/fetch-rosemary.sh "$DIR/rosemary"
```

Create `"$DIR/flags.json"` with the local flag before the first start:

```json
{
  "tbxFlagEnableExperimentalMacClient": true,
  "tbxFlagDisableUpdater": true
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

Keep the existing `compat/`, `libtuxblox/` and `runtime/` directories. Add the flag to `~/.tuxblox/flags.json`, keeping any entries already there:

```json
{
  "tbxFlagEnableExperimentalMacClient": true,
  "tbxFlagDisableUpdater": true
}
```

Restart the launcher. Without the first flag the Player card follows TuxBlox's normal state; Studio is unaffected.

`tbxFlagDisableUpdater` is recommended while this is a separate build: the official updater or installer can replace this launcher with the upstream one. Do not run the official installer over this launcher.

## Updates

The launcher carries TuxBlox's updater. Until it has its own release channel, keep `tbxFlagDisableUpdater` set in `flags.json` while testing.

Discord Rich Presence and server-region settings are read when a Player session starts. Restart the Player after changing them.

## Troubleshooting

- **Studio says the compatibility layer is missing.** This is the Player-only layout. Install TuxBlox and use its directory if you want Studio.
- **The Player closes right after opening.** Check that `lms.roblox.com` resolves normally. Some DNS filters return `0.0.0.0` for it; the macOS client cannot finish its login flow without that endpoint. `www.roblox.com` and `clientsettings.roblox.com` being reachable is not enough.
- **The Player does not start.** Confirm that `<install>/rosemary/rosemary` exists and is executable, or set `TUXBLOX_ROSEMARY` to the binary.

## Licensing

TuxBlox's own code is GPLv3. The compatibility layer in TuxBlox stays LGPLv2.1, with `compat/tuxblox/` and `compat/webkitgtk/bundle/` under GPLv3; it is not part of this repository. Rosemary is a separate GPL-3.0 executable and is not linked into the launcher; its licence is in `third_party_licenses/rosemary/LICENSE`.

The launcher runs the official, unmodified Roblox clients. It is not affiliated with, endorsed by, or associated with Roblox Corporation.
