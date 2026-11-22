<p align="center">
  <img src="https://tuxblox.net/static/images/banner/tuxblox-banner.png" width="600">
</p>

***

# TuxBlox 🐧

TuxBlox is a free, open-source compatibility layer and launcher that runs Roblox on Linux. It's designed for both beginners and experts, and is under **active development**.

## How It Works

TuxBlox doesn't emulate an operating system or use a virtual machine. It runs the official macOS version of Roblox on Linux by translating its Darwin API calls at runtime.

TuxBlox maintains its own Rosemary build (based on Rosemary v0.5.0) made specifically for Roblox, and bundles it. You don't need Rosemary installed on your system. Targeting a single application lets us fix problems at the source, including upstream Rosemary bugs that affect Roblox, instead of working around them with configuration.

TuxBlox runs the **official, unmodified** Roblox binaries. It never patches or repackages them, and it refuses to run files that have been modified.

## Features

> [!NOTE]
> **TuxBlox has some issues with Roblox Studio.** We are aiming to fix these issues. Updates will be posted on our [Discord](https://tuxblox.net/discord).

> [!NOTE]
> **TuxBlox previously used Wine to run Roblox.** Some documentation may still be out of date, and the macOS runtime is still in public betatesting, so stable releases are not affected yet.

* **Works out of the box:** installation, updates and graphics setup are handled for you. There are no launch options to supply and no prefix to maintain.
* **Session manager:** see each running session's memory use, process ID, Roblox version and logs, and stop sessions.
* **Version picker:** download any Roblox version you choose, straight from Roblox's servers.
* **FastFlag manager.**
* **Release channels:** switch between stable, canary and experimental, with automatic updates.
* **Full control:** every default the launcher applies can be changed in its settings.

## System Requirements

### Hardware

* **Processor:** x86-64 with SSE3 support or newer (ARM64 is planned)
* **Memory:** 4 GB of RAM
* **Disk space:** 4 GB free
* **Graphics:** a GPU with Vulkan 1.1 drivers

### Software

* **Kernel:** Linux 5.6 or newer
* **C library:** glibc 2.30 or newer
* **Display:** X11 or Wayland

On a Wayland session Studio runs through XWayland, because Wayland gives a client no way to place its own windows and Studio's docked panels depend on that. Set `DFFlagPreferXWayland` to prefer XWayland for everything else too.

TuxBlox installs and runs as a normal user. Root is never required.

## Installation

### 1. Install from the Releases page
We distribute TuxBlox files here in [GitHub releases](https://github.com/tuxblox/tuxblox/releases) and [tuxblox.net/releases](https://tuxblox.net/releases). Head over to whichever page you prefer, then download the `TuxBloxInstaller` binary.

Make sure to set `chmod +x` on the installer:
```bash
chmod +x TuxBloxInstaller
```
Then run the installer.

We also distribute the `.AppImage` variant of the installer. The steps to download and run are the exact same.

### 2. Install from the command line

This will automatically install the latest stable version of TuxBlox for you. Open a terminal and run:

```bash
curl -fsSL https://tuxblox.net/install.sh | bash
```

Feel free to inspect the script first:

```bash
curl -fsSL https://tuxblox.net/install.sh -o install.sh
less install.sh
bash install.sh
```

### 3. Install the Flatpak
We also release Flatpak versions of TuxBlox, however **they are very experimental and Flatpak's sandbox might break TuxBlox**. We recommend using the releases page or the command-line.

Head over to [GitHub Releases](https://github.com/tuxblox/tuxblox/releases) or [tuxblox.net/releases](https://tuxblox.net/releases), and download `net.tuxblox.TuxBlox.flatpak`.

Then open a terminal, and go to your Downloads folder:
```bash
cd ~/Downloads # change this to where you downloaded the flatpak to
flatpak install --user ./net.tuxblox.TuxBlox.flatpak
```
Note that the Flatpak releases do not automatically update, so you'll need to manually reinstall the Flatpak every time.

## Building From Source

For a full build, run `make`. For the Flatpak only, run `make flatpak`.
Full documentation for building from source and the requirements can be seen here: https://tuxblox.net/docs/development/building-from-source

## Privacy

Some features contact TuxBlox's own servers: self-updates, the optional server-location lookup for Team Create, and optional crash reports. Roblox itself is always downloaded from Roblox's servers, and your Roblox account cookie is never sent to TuxBlox. See our [privacy policy](https://tuxblox.net/privacy).

## FAQ

**Is TuxBlox a fork of Vinegar or Sober?**

No, TuxBlox is its own project. Vinegar runs the Windows client through the Wine you install yourself, while TuxBlox runs the macOS client through a compatibility layer it bundles and builds for Roblox. Sober runs the Android version of Roblox instead.

**Does it work on Wayland?**

Yes. The launcher runs natively on Wayland, and Studio runs through XWayland.

**Why isn't there a distro package?**

TuxBlox updates itself whenever a release is published, which is not how a distribution's package manager expects to work.

**Can I run other macOS apps with it?**

No. TuxBlox only runs programs signed by Roblox or by TuxBlox itself.

**Is it free?**

Yes. TuxBlox is free and open source under GPL-3.0-or-later.

Additional questions are answered in [tuxblox.net/docs/getting-started/faq](https://tuxblox.net/docs/getting-started/faq).

## Contributing

Contributions are welcome! Read [CONTRIBUTING.md](CONTRIBUTING.md) before opening a pull request. If you use AI tools, also read [AGENTS.md](AGENTS.md) for the repository's AI policy.

To report a security vulnerability, **don't open a public issue**. See [SECURITY.md](SECURITY.md).

## Any Questions?

Join our [Discord server](https://tuxblox.net/discord) for support, development updates and discussion, or read the [documentation](https://tuxblox.net/docs).

## Legal & Disclaimer

- **TuxBlox is an independent, unofficial community project.** It is not affiliated with, authorized or endorsed by Roblox Corporation. Roblox is a trademark of Roblox Corporation.
- **TuxBlox's own code is licensed under GPL-3.0-or-later.**
- **This project includes a modified version of Microlift's [Rosemary](https://github.com/microlift/rosemary) which is also licensed under GPL-3.0-or-later.
- See [`third_party_licenses/`](third_party_licenses/) for the full licence text of all third-party code in this repository.