# TuxBlox Mac

A small, standalone launcher for Roblox's official macOS Player on Linux, running through Rosemary.

This is a separate project from TuxBlox. TuxBlox stays focused on the Windows client and its compatibility layer; this launcher is for the macOS Player, which is useful while the Windows Player path is still in progress and for machines where the macOS build is a better fit.

## Status

Early development. The first version is a single-window GTK launcher that installs, verifies, launches and stops the official macOS Player through Rosemary.

## What it will do

- download the official `RobloxPlayer.zip` from Roblox's setup CDN;
- verify the Apple code signature against a pinned Apple Root CA;
- install the unmodified app under the launcher's data directory, with an integrity manifest;
- start it through Rosemary, with launch and stop controls and a desktop entry;
- keep the client unpatched: the only file written inside the bundle is `ClientSettings` for FastFlags;
- support x86_64 and arm64 hosts, with the client architecture following the host.

There is no Windows or Studio support here. Those stay in TuxBlox.

Discord Rich Presence, server-region notifications, desktop entries and the remaining settings come after the first working launcher.

## Requirements

- Linux x86_64 (arm64 packaging is next)
- Rosemary (`rosemary` on `PATH`, or `<data-dir>/rosemary/rosemary`)
- OpenSSL, libcurl, libarchive and zlib for the backend
- GTK4/libadwaita for the launcher interface

## Building

```sh
cmake -S . -B build -G Ninja
cmake --build build
./build/tuxblox-mac
```

The client is kept in `~/.local/share/tuxblox-mac` by default; set `TUXBLOX_MAC_DIR` to use another directory. Rosemary is found through `TUXBLOX_ROSEMARY`, `<data-dir>/rosemary/rosemary`, or `PATH`.

The first launch downloads the official Player from Roblox's setup CDN and verifies its Apple signature before installing it. Nothing is patched.

## Licensing

This project's own code is GPLv3. Rosemary is a separate GPL-3.0 executable and is not linked into this program; its licence is in `third_party_licenses/rosemary/LICENSE`.

The launcher runs the official, unmodified Roblox client. It is not affiliated with, endorsed by, or associated with Roblox Corporation.
