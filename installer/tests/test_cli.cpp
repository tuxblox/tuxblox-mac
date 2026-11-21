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

#include "cli.h"
#include <cassert>
#include <cstdio>
#include <initializer_list>
#include <string>
#include <vector>

using tuxblox::CliOptions;
using tuxblox::parseArgs;

namespace {
CliOptions parse(std::initializer_list<const char*> args) {
    std::vector<const char*> argv;
    argv.push_back("TuxBloxInstaller");
    for (const char* a : args) argv.push_back(a);
    return parseArgs(static_cast<int>(argv.size()), argv.data());
}
} // namespace

int main() {
    {
        CliOptions o = parse({});
        assert(!o.uninstall && !o.headless && !o.noLaunch && !o.help);
        assert(o.channel == "stable");
        assert(o.release.empty() && o.releaseFile.empty());
        assert(o.error.empty());
    }
    {
        CliOptions o = parse({"--headless"});
        assert(o.headless && !o.noLaunch && o.error.empty());
    }
    {
        CliOptions o = parse({"--nolaunch"});
        assert(o.noLaunch && !o.headless && o.error.empty());
    }
    {
        CliOptions o = parse({"--uninstall"});
        assert(o.uninstall && o.error.empty());
    }
    {
        CliOptions o = parse({"--help"});
        assert(o.help && o.error.empty());
        CliOptions s = parse({"-h"});
        assert(s.help && s.error.empty());
    }
    {
        CliOptions o = parse({"--latest"});
        assert(o.error.empty());
    }
    {
        CliOptions o = parse({"--channel", "beta"});
        assert(o.channel == "beta");
        assert(o.error.empty());
    }
    {
        CliOptions o = parse({"--nolaunch", "--channel", "beta", "--headless"});
        assert(o.headless && o.noLaunch);
        assert(o.channel == "beta");
        assert(o.error.empty());
    }
    {
        CliOptions o = parse({"--release", "3.6.0.1", "--channel", "canary"});
        assert(o.release == "3.6.0.1");
        assert(o.channel == "canary");
        assert(o.error.empty());
    }
    {
        CliOptions o = parse({"--headless", "--uninstall"});
        assert(o.uninstall && o.headless && o.error.empty());
    }
    {
        CliOptions o = parse({"--channel", "dev"});
        assert(o.error.empty());
        assert(o.channel == "experimental");
    }
    {
        CliOptions o = parse({"--channel", "development"});
        assert(o.channel == "development");
    }
    {
        CliOptions o = parse({"--channel"});
        assert(!o.error.empty());
    }
    {
        CliOptions o = parse({"--nolanch"});
        assert(!o.error.empty());
        assert(o.error.find("--nolanch") != std::string::npos);
    }
    {
        CliOptions o = parse({"install"});
        assert(!o.error.empty());
    }
    {
        CliOptions o = parse({"--dir", "/opt/tuxblox"});
        assert(o.error.empty());
        assert(o.dir == "/opt/tuxblox");
    }
    {
        CliOptions o = parse({"--dir", "/opt/tuxblox/"});
        assert(o.error.empty() && o.dir == "/opt/tuxblox");
    }
    {
        CliOptions o = parse({"--dir", "/opt//tuxblox/./"});
        assert(o.error.empty() && o.dir == "/opt/tuxblox");
    }
    {
        CliOptions o = parse({"--dir", "/"});
        assert(o.error.empty() && o.dir == "/");
    }
    {
        CliOptions o = parse({"--dir", "relative/path"});
        assert(!o.error.empty());
    }
    {
        CliOptions o = parse({"--dir"});
        assert(!o.error.empty());
    }
    {
        CliOptions o = parse({"--dir", "/opt/tuxblox", "--headless"});
        assert(o.error.empty() && o.dir == "/opt/tuxblox" && o.headless);
    }
    {
        CliOptions o = parse({"--headless", "--dir", "/opt/tuxblox"});
        assert(o.error.empty() && o.dir == "/opt/tuxblox" && o.headless);
    }
    {
        CliOptions o = parse({"--headless"});
        assert(o.error.empty() && o.dir.empty());
    }
    {
        CliOptions o = parse({"--language", "pt_BR"});
        assert(o.error.empty() && o.language == "pt_BR");
    }
    {
        CliOptions o = parse({"--language", "fr"});
        assert(!o.error.empty() && o.language == "fr");
    }
    {
        CliOptions o = parse({"--language"});
        assert(!o.error.empty());
    }
    {
        CliOptions o = parse({"--allow-root"});
        assert(o.error.empty() && o.allowRoot);
    }
    {
        CliOptions o = parse({"--headless"});
        assert(o.error.empty() && !o.allowRoot);
    }
    {
        const std::string usage = tuxblox::usageText();
        assert(usage.find("--headless") != std::string::npos);
        assert(usage.find("--nolaunch") != std::string::npos);
        assert(usage.find("--uninstall") != std::string::npos);
        assert(usage.find("--channel") != std::string::npos);
        assert(usage.find("--latest") != std::string::npos);
        assert(usage.find("--dir") != std::string::npos);
        assert(usage.find("--allow-root") != std::string::npos);
        assert(usage.find("--language") != std::string::npos);
    }

    printf("cli: all tests passed\n");
    return 0;
}
