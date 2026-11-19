# TuxBlox Mac

The complete TuxBlox launcher, with the experimental macOS Player backend, kept in a separate repository.

The Windows Studio path is inherited from TuxBlox and is not rewritten here. Studio continues to use the same compatibility layer and data as a normal TuxBlox install; the macOS Player runs through Rosemary and lives beside it.

## What is here

This tree is the launcher half of TuxBlox: the full interface (Home, Sessions, Versions, FastFlags, Settings and About), the shared UI, flags, crypto, release and translation code, and the assets.

It deliberately does **not** contain `compat/`, the Wine-based Windows compatibility layer. Studio is unchanged because it keeps using the layer already installed by TuxBlox.

## Installing

The launcher finds its data from the directory it is installed in, just like TuxBlox. To keep Studio working, install the built `TuxBloxLauncher` into the existing TuxBlox directory (usually `~/.tuxblox`) and start it from there:

```sh
install -m 755 build/launcher/TuxBloxLauncher ~/.tuxblox/TuxBloxLauncher
~/.tuxblox/TuxBloxLauncher
```

The macOS Player is kept under `<install>/mac-client`, and the launcher looks for Rosemary at `TUXBLOX_ROSEMARY`, `<install>/rosemary/rosemary`, or `rosemary` on `PATH`.

## Building

The launcher builds with the same toolchain as TuxBlox. The Windows compatibility layer is not built from this tree, so the launcher target is the one to use:

```sh
make launcher
```

The repository includes the `Makefile`, launcher, shared UI and assets needed for that target. The same build requirements as TuxBlox apply: GTK4/libadwaita, libcurl, libarchive, OpenSSL, zlib, CMake and Ninja.

The macOS Player and the Windows Studio path both run the official, unmodified clients. Nothing here patches them.

## Updates

This launcher carries TuxBlox's updater. Until it has its own release channel, the official updater can replace it with the upstream launcher. Keep `tbxFlagDisableUpdater` set in `flags.json` while testing, or do not run the official installer over this launcher.

## Licensing

TuxBlox's own code is GPLv3. The compatibility layer in TuxBlox stays LGPLv2.1, with `compat/tuxblox/` and `compat/webkitgtk/bundle/` under GPLv3; it is not part of this repository. Rosemary is a separate GPL-3.0 executable and is not linked into the launcher; its licence is in `third_party_licenses/rosemary/LICENSE`.

The launcher runs the official, unmodified Roblox clients. It is not affiliated with, endorsed by, or associated with Roblox Corporation.
