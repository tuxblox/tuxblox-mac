// TuxBlox - Linux Compatibility Layer for the Roblox Engine
// Copyright (C) 2026 TuxBlox Developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include "install_paths.h"
#include "launch_paths.h"
#include "lnk_resolver.h"
#include "mcp_locate.h"
#include "release_stamp.h"
#include "version.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;
using namespace tuxblox;

namespace {

} // namespace

int main(int argc, char** argv) {
    std::vector<std::string> passthrough;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--version") {
            printf("%s\n", reportedVersion(kTuxBloxBuildId).c_str());
            return 0;
        }
        if (arg == "--component-version") {
            printf("%s\n", kTuxBloxVersion);
            return 0;
        }
        passthrough.push_back(arg);
    }

    std::string dir;
    try {
        dir = installDir();
    } catch (const std::exception& e) {
        fprintf(stderr, "TuxBlox: %s\n", e.what());
        return 1;
    }

    const std::string compat = compatBinaryPath(dir);
    std::error_code ec;
    if (!fs::exists(compat, ec) || ec) {
        fprintf(stderr, "TuxBlox: the compatibility layer is missing from %s.\n"
                        "Open TuxBlox once to finish installing it.\n",
                dir.c_str());
        return 1;
    }

    const std::string mcpExe = findStudioMcp(dir);
    if (mcpExe.empty()) {
        fprintf(stderr,
                "TuxBlox: Roblox's Studio MCP server is not installed.\n"
                "Install it by following Roblox's own guide at\n"
                "https://create.roblox.com/docs/studio/mcp, with Studio started\n"
                "through TuxBlox at least once first.\n");
        return 1;
    }

    for (const std::string& kv : launchEnvVars(dir, LaunchTarget::Studio)) {
        const size_t split = kv.find('=');
        setenv(kv.substr(0, split).c_str(), kv.substr(split + 1).c_str(), 1);
    }

    setenv("TUXBLOX_LOG", "-1", 1);

    std::vector<std::string> args = {compat, "run", "--immediate"};
    args.push_back(mcpExe);
    for (const std::string& arg : passthrough) args.push_back(arg);

    std::vector<char*> raw;
    raw.reserve(args.size() + 1);
    for (std::string& arg : args) raw.push_back(const_cast<char*>(arg.c_str()));
    raw.push_back(nullptr);

    execv(compat.c_str(), raw.data());

    fprintf(stderr, "TuxBlox: could not start the compatibility layer at %s\n", compat.c_str());
    return 1;
}
