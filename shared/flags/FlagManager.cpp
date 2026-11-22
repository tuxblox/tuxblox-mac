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

#include "FlagManager.h"

#include "json.hpp"
#include "tuxblox_default_flags.h"
#include <array>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <unistd.h>
#include <mutex>
#include <optional>
#include <sstream>
#include <thread>

namespace tuxblox {

namespace {

constexpr std::array<std::pair<const char*, FlagType>, 3> kTypeWords = {{
    {"Flag", FlagType::Bool},
    {"String", FlagType::String},
    {"Int", FlagType::Int},
}};

constexpr std::array<std::pair<const char*, const char*>, 1> kBaked = {{
    {"FStringFlagsVersion", "2.0.0"},
}};

struct FlagName {
    bool synchronized = false;
    bool dynamic = false;
    FlagType type = FlagType::Bool;
};

std::optional<FlagName> parseFlagName(const std::string& name) {
    FlagName out;
    size_t at = 0;
    if (at < name.size() && name[at] == 'S') { out.synchronized = true; ++at; }
    if (at < name.size() && name[at] == 'D') { out.dynamic = true; ++at; }
    if (at >= name.size() || name[at] != 'F') return std::nullopt;
    ++at;
    for (const auto& entry : kTypeWords) {
        const std::string word = entry.first;
        if (name.size() > at + word.size() && name.compare(at, word.size(), word) == 0) {
            out.type = entry.second;
            return out;
        }
    }
    return std::nullopt;
}

std::optional<FlagValue> bakedFlag(const std::string& name) {
    for (const auto& entry : kBaked) {
        if (name != entry.first) continue;
        FlagValue value;
        value.type = FlagType::String;
        value.text = entry.second;
        return value;
    }
    return std::nullopt;
}

}

std::optional<FlagType> flagTypeForName(const std::string& name) {
    const std::optional<FlagName> parsed = parseFlagName(name);
    if (!parsed.has_value()) return std::nullopt;
    return parsed->type;
}

bool isLocalOnlyFlag(const std::string& name) {
    const std::optional<FlagName> parsed = parseFlagName(name);
    return !parsed.has_value() || !parsed->synchronized;
}

bool isBakedFlag(const std::string& name) { return bakedFlag(name).has_value(); }

FlagTable parseFlagJson(const std::string& json, bool fromServer) {
    FlagTable table;
    nlohmann::json parsed;
    try {
        parsed = nlohmann::json::parse(json);
    } catch (const nlohmann::json::exception& failure) {
        if (!json.empty()) {
            fprintf(stderr, "TuxBlox: ignoring flags that are not valid JSON (%s)\n", failure.what());
        }
        return table;
    }
    if (!parsed.is_object()) {
        fprintf(stderr, "TuxBlox: ignoring flags that are not a JSON object\n");
        return table;
    }

    for (auto it = parsed.begin(); it != parsed.end(); ++it) {
        const std::optional<FlagType> type = flagTypeForName(it.key());
        if (!type.has_value()) {
            fprintf(stderr, "Warning: unrecognized flag '%s'.\n", it.key().c_str());
            continue;
        }
        if (isBakedFlag(it.key())) {
            fprintf(stderr, "TuxBlox: ignoring the flag \"%s\": its value is compiled in\n",
                    it.key().c_str());
            continue;
        }
        if (fromServer && isLocalOnlyFlag(it.key())) {
            fprintf(stderr, "TuxBlox: ignoring the flag \"%s\" from the server: it can only be "
                            "set on this machine\n", it.key().c_str());
            continue;
        }

        FlagValue value;
        value.type = *type;
        bool usable = true;
        if (*type == FlagType::Bool) {
            if (!it->is_boolean()) usable = false;
            else value.boolean = it->get<bool>();
        } else if (*type == FlagType::String) {
            if (!it->is_string()) usable = false;
            else value.text = it->get<std::string>();
        } else {
            if (!it->is_number_integer() || it->is_boolean()) usable = false;
            else if (it->is_number_unsigned() &&
                     it->get<uint64_t>() > static_cast<uint64_t>(INT64_MAX)) usable = false;
            else value.integer = it->get<int64_t>();
        }
        if (!usable) {
            fprintf(stderr, "TuxBlox: ignoring the flag \"%s\": its value is not the kind its "
                            "name says\n", it.key().c_str());
            continue;
        }
        table[it.key()] = value;
    }
    return table;
}

void mergeFlagsOver(FlagTable& base, const FlagTable& higher) {
    for (const auto& entry : higher) base[entry.first] = entry.second;
}

namespace {

const char* const kConfigBase = "https://config.tuxblox.net";
constexpr long kMaxBody = 64 * 1024;

struct Pending {
    std::mutex mutex;
    std::condition_variable done;
    bool finished = false;
    std::string body;
};

struct State {
    FlagTable table;
    std::shared_ptr<Pending> pending;
    std::string installDir;
    int timeoutSeconds = 5;
    bool sealed = false;
    bool initialised = false;
};

std::mutex& stateMutex() {
    static std::mutex m;
    return m;
}

State& state() {
    static State s;
    return s;
}

FlagTable defaultTable() {
    return parseFlagJson(kTuxBloxDefaultFlagsJson, false);
}

std::string readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return {};
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

FlagTable flagsFromEnvironment() {
    FlagTable table;
    for (char** entry = environ; entry != nullptr && *entry != nullptr; ++entry) {
        const std::string text = *entry;
        const size_t eq = text.find('=');
        if (eq == std::string::npos) continue;
        const std::string name = text.substr(0, eq);
        const std::optional<FlagType> type = flagTypeForName(name);
        if (!type.has_value()) continue;
        if (isBakedFlag(name)) {
            fprintf(stderr, "TuxBlox: ignoring the flag \"%s\": its value is compiled in\n",
                    name.c_str());
            continue;
        }
        const std::string text_value = text.substr(eq + 1);
        FlagValue value;
        value.type = *type;
        if (*type == FlagType::Bool) {
            std::string lowered = text_value;
            for (char& c : lowered) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
            if (lowered == "true" || lowered == "1") {
                value.boolean = true;
            } else if (lowered == "false" || lowered == "0") {
                value.boolean = false;
            } else {
                fprintf(stderr, "Warning: flag '%s' is not a boolean.\n", name.c_str());
                continue;
            }
        } else if (*type == FlagType::String) {
            value.text = text_value;
        } else {
            try {
                size_t used = 0;
                value.integer = std::stoll(text_value, &used);
                if (used != text_value.size()) throw std::invalid_argument("trailing");
            } catch (const std::exception&) {
                fprintf(stderr, "Warning: flag '%s' is not an integer.\n", name.c_str());
                continue;
            }
        }
        table[name] = value;
    }
    return table;
}

void sealLocked(State& s) {
    if (s.sealed) return;
    s.sealed = true;

    if (s.pending) {
        std::unique_lock<std::mutex> lock(s.pending->mutex);
        s.pending->done.wait_until(lock, std::chrono::system_clock::now() + std::chrono::seconds(s.timeoutSeconds),
                                   [&] { return s.pending->finished; });
        if (s.pending->finished && !s.pending->body.empty()) {
            mergeFlagsOver(s.table, parseFlagJson(s.pending->body, true));
        }
    }
    if (!s.installDir.empty()) {
        const std::string override = readFile(s.installDir + "/flags.json");
        if (!override.empty()) mergeFlagsOver(s.table, parseFlagJson(override, false));
    }
    mergeFlagsOver(s.table, flagsFromEnvironment());
}

std::optional<FlagValue> lookup(const std::string& name) {
    if (const std::optional<FlagValue> baked = bakedFlag(name)) return baked;
    std::lock_guard<std::mutex> lock(stateMutex());
    State& s = state();
    if (s.table.empty() && !s.sealed) s.table = defaultTable();
    sealLocked(s);
    auto it = s.table.find(name);
    if (it == s.table.end()) return std::nullopt;
    return it->second;
}

}

const char* flagArch() {
#if defined(__aarch64__)
    return "arm64";
#else
    return "x86_64";
#endif
}

std::string clientFlagsUrl(const std::string& channel, const std::string& arch) {
    return std::string(kConfigBase) + "/v1/tuxblox/client/" + channel + "/" + arch + "/flags";
}

void resetFlags() {
    std::lock_guard<std::mutex> lock(stateMutex());
    State& s = state();
    s.table.clear();
    s.pending.reset();
    s.installDir.clear();
    s.sealed = false;
    s.initialised = false;
}

void initFlags(const std::string& installDir, const std::string& channel,
               bool isRecordedRelease, int timeoutSeconds,
               FlagFetch fetch, const std::string& baseUrl) {
    std::lock_guard<std::mutex> lock(stateMutex());
    State& s = state();
    if (s.initialised || s.sealed) {
        fprintf(stderr, "TuxBlox: flags are already loaded; this call is ignored\n");
        return;
    }
    s.initialised = true;
    s.table = defaultTable();
    s.installDir = installDir;
    s.timeoutSeconds = timeoutSeconds > 0 ? timeoutSeconds : 5;
    s.sealed = false;
    s.pending.reset();

    if (!isRecordedRelease) return;

    const std::string base = baseUrl.empty() ? kConfigBase : baseUrl;
    const std::string url = base + "/v1/tuxblox/client/" + channel + "/" + flagArch() + "/flags";
    auto pending = std::make_shared<Pending>();
    s.pending = pending;
    if (!fetch) return;
    FlagFetch run = fetch;

    std::thread([pending, run, url, seconds = s.timeoutSeconds] {
        std::string body;
        try {
            body = run(url, seconds);
        } catch (...) {
        }
        {
            std::lock_guard<std::mutex> guard(pending->mutex);
            pending->body = std::move(body);
            pending->finished = true;
        }
        pending->done.notify_all();
    }).detach();
}

void setFlag(const std::string& name, const FlagValue& value) {
    const std::optional<FlagName> parsed = parseFlagName(name);
    if (!parsed.has_value() || !parsed->dynamic) {
        fprintf(stderr, "TuxBlox: refusing to set the flag \"%s\" at runtime: only a dynamic flag may change\n",
                name.c_str());
        abort();
    }
    if (parsed->type != value.type) {
        fprintf(stderr, "TuxBlox: refusing to set the flag \"%s\" at runtime: its value contradicts its name\n",
                name.c_str());
        abort();
    }
    std::lock_guard<std::mutex> lock(stateMutex());
    State& s = state();
    if (s.table.empty() && !s.sealed) s.table = defaultTable();
    s.table[name] = value;
}

bool flagBool(const std::string& name) {
    const std::optional<FlagValue> value = lookup(name);
    return value && value->type == FlagType::Bool && value->boolean;
}

bool flagBoolOffline(const std::string& installDir, const std::string& name) {
    FlagTable table = defaultTable();
    if (!installDir.empty()) {
        mergeFlagsOver(table, parseFlagJson(readFile(installDir + "/flags.json"), false));
    }
    const auto found = table.find(name);
    return found != table.end() && found->second.type == FlagType::Bool && found->second.boolean;
}

int64_t flagInt(const std::string& name) {
    const std::optional<FlagValue> value = lookup(name);
    return value && value->type == FlagType::Int ? value->integer : 0;
}

std::string flagString(const std::string& name) {
    const std::optional<FlagValue> value = lookup(name);
    return value && value->type == FlagType::String ? value->text : std::string();
}

}
