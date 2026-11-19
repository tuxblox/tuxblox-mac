# TuxBlox Mac

The complete TuxBlox launcher, with the experimental macOS Player backend and the Studio path inherited from TuxBlox.

This repository is the launcher half of TuxBlox. The Windows Studio path is unchanged: it keeps using the compatibility layer from an existing TuxBlox install. The macOS Player runs through Rosemary and lives beside it. The `compat/` layer is intentionally not duplicated here.

## Status

- The complete launcher UI is here: Home, Sessions, Versions, FastFlags, Settings and About.
- The macOS Player backend is experimental and sits behind a local-only flag.
- Studio is inherited unchanged and still uses the TuxBlox compatibility layer.
- Tested with the TuxBlox `3.12.15.2` install; builds are x86_64 for now. arm64 is not supported yet.

## What it does

- Downloads the official macOS Player from Roblox's setup CDN.
- Verifies the Apple code signature against the pinned Apple Root CA before installing anything.
- Installs the unmodified bundle under `<install>/mac-client` with a local integrity manifest.
- Starts it through Rosemary, with the same launcher, Sessions, settings, Discord Rich Presence and server-region features as TuxBlox.
- Keeps Studio on the Windows compatibility layer, with no changes to its code path.

Nothing patches either client. The only file written inside the macOS bundle is `ClientSettings` for FastFlags.

## Requirements

- Linux x86_64.
- An existing TuxBlox install (`~/.tuxblox` by default) with its compatibility layer, for Studio.
- Rosemary: `TUXBLOX_ROSEMARY`, `<install>/rosemary/rosemary`, or `rosemary` on `PATH`.
- Build requirements: GTK4/libadwaita, libcurl, libarchive, OpenSSL, zlib, CMake and Ninja. The `Makefile` uses the same build container as TuxBlox.

## Enabling the macOS Player

The backend is deliberately local-only, so the config server cannot switch an install onto it. Add the flag to `<install>/flags.json`, keeping any entries already there:

```json
{
  "tbxFlagEnableExperimentalMacClient": true,
  "tbxFlagDisableUpdater": true
}
```

Restart the launcher. Without the first flag the Player card follows TuxBlox's normal state; Studio is unaffected. `tbxFlagDisableUpdater` is recommended while this is a separate build, as explained under [Updates](#updates).

## Building

The repository contains the launcher, shared UI, flags, crypto, i18n, assets and the build files, but not `compat/`. Build the launcher target:

```sh
TUXBLOX_DONT_INCREMENT_BUILD=1 make launcher
```

The launcher target builds its shared libraries and UI stack first, using the TuxBlox build container. The result is:

```text
build/native/TuxBloxLauncher
```

The same toolchain and container requirements as the TuxBlox launcher apply. The Windows compatibility layer is not built from this tree.

## Installing

The launcher finds its data from the directory it is installed in. Install it into the existing TuxBlox directory so Studio keeps its compatibility layer:

```sh
install -m 755 build/native/TuxBloxLauncher ~/.tuxblox/TuxBloxLauncher
~/.tuxblox/TuxBloxLauncher
```

The macOS Player is kept under `<install>/mac-client`. The first launch downloads it; later launches check and repair the installed build.

## Updates

This launcher carries TuxBlox's updater. Until it has its own release channel, the official updater or installer can replace it with the upstream launcher. Keep `tbxFlagDisableUpdater` set in `flags.json` while testing, and do not run the official installer over this launcher.

Discord Rich Presence and server-region settings are read when a Player session starts. Restart the Player after changing them.

## Licensing

TuxBlox's own code is GPLv3. The compatibility layer in TuxBlox stays LGPLv2.1, with `compat/tuxblox/` and `compat/webkitgtk/bundle/` under GPLv3; it is not part of this repository. Rosemary is a separate GPL-3.0 executable and is not linked into the launcher; its licence is in `third_party_licenses/rosemary/LICENSE`.

The launcher runs the official, unmodified Roblox clients. It is not affiliated with, endorsed by, or associated with Roblox Corporation.
