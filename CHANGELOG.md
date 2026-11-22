This is the biggest release in TuxBlox's history.

## Roblox Player support is finally here!
Almost 5 months in, TuxBlox can now run the Roblox Player.

Getting here took a big architectural change. Since 2023, the Windows client has shipped with Hyperion, an anti-tamper that makes running it outside real Windows extremely hard to maintain. Instead of fighting that, TuxBlox now runs the macOS Roblox client, using our own fork of Rosemary, a compatibility layer for running macOS apps on Linux. Our Wine fork has been retired.

### What this means for you:

* Roblox Player works, with real desktop graphics, desktop integration and desktop-only games, unlike the Android client
* The compatibility layer is now a single binary, smaller than the launcher itself, so downloads and updates are much faster
* arm64 support is next. Roblox distributes native arm64 macOS builds, so no emulation is needed

## Known issues:
* Roblox Studio has bugs under the new runtime. Studio fixes are in progress.
* The documentation is out-of-date. The documentation will update once this runtime reaches stable releases.

Since Studio has some bugs, we didn't distribute the macOS runtime to stable and canary channels yet, the macOS runtime is now in public beta testing. 

If you prefer a stable Studio experience, do not update to this version yet. However, we would appreciate it if y'all test the runtime and report bugs.
To receive this update, switch to the experimental channel in Settings -> Updates -> Update channel. Report bugs in GitHub issues and follow the bug reporting instructions.