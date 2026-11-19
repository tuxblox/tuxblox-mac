# TuxBlox Mac

A small, standalone launcher for Roblox's official macOS Player on Linux, running through Rosemary.

This is a separate project from TuxBlox. TuxBlox stays focused on the Windows client and its compatibility layer; this launcher is for the macOS Player, which is useful while the Windows Player path is still in progress and for machines where the macOS build is a better fit.

## Status

Early development. The macOS backend is being moved here from the experimental TuxBlox branch, and the standalone interface is being built around it.

## What it will do

- download the official `RobloxPlayer.zip` from Roblox's setup CDN;
- verify the Apple code signature against a pinned Apple Root CA;
- install the unmodified app under the launcher's data directory, with an integrity manifest;
- start it through Rosemary, with launch and stop controls and a desktop entry;
- keep the client unpatched: the only file written inside the bundle is `ClientSettings` for FastFlags;
- support x86_64 and arm64 hosts, with the client architecture following the host.

There is no Windows or Studio support here. Those stay in TuxBlox.

Discord Rich Presence, server-region notifications and the remaining settings come after the first working launcher.

## Requirements

- Linux x86_64 or arm64
- Rosemary (`rosemary` on `PATH`, or a pinned copy managed by the launcher)
- OpenSSL, libcurl, libarchive and zlib for the backend
- GTK4/libadwaita for the launcher interface

Build instructions will be added with the first source import.

## Licensing

This project's own code is GPLv3. Rosemary is a separate GPL-3.0 executable and is not linked into this program; its licence is in `third_party_licenses/rosemary/LICENSE`.

The launcher runs the official, unmodified Roblox client. It is not affiliated with, endorsed by, or associated with Roblox Corporation.
