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

#pragma once
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>

namespace tuxblox {

using FlagFetch = std::function<std::string(const std::string& url, int timeoutSeconds)>;

const char* flagArch();

std::string clientFlagsUrl(const std::string& channel, const std::string& arch);

void initFlags(const std::string& installDir, const std::string& channel,
               bool isRecordedRelease, int timeoutSeconds,
               FlagFetch fetch = {}, const std::string& baseUrl = {});

void resetFlags();

bool flagBool(const std::string& name);
int64_t flagInt(const std::string& name);
std::string flagString(const std::string& name);

bool flagBoolOffline(const std::string& installDir, const std::string& name);

enum class FlagType { Bool, String, Int };

struct FlagValue {
    FlagType type = FlagType::Bool;
    bool boolean = false;
    int64_t integer = 0;
    std::string text;
};

using FlagTable = std::map<std::string, FlagValue>;

void setFlag(const std::string& name, const FlagValue& value);

std::optional<FlagType> flagTypeForName(const std::string& name);

bool isLocalOnlyFlag(const std::string& name);

FlagTable parseFlagJson(const std::string& json, bool fromServer);

void mergeFlagsOver(FlagTable& base, const FlagTable& higher);

}
