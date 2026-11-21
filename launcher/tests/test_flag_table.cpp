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
#include <cassert>
#include <cstdio>
#include <string>

int main() {
    using namespace tuxblox;

    // The prefix states the type, as Roblox's own flags do.
    {
        assert(flagTypeForName("SFFlagEnableSeasonalTheme").value() == FlagType::Bool);
        assert(flagTypeForName("SFStringPrivacyPolicyVersion").value() == FlagType::String);
        assert(flagTypeForName("SFIntRobloxInstallerParallelWorkers").value() == FlagType::Int);
        assert(!flagTypeForName("somethingElse").has_value());
        assert(!flagTypeForName("FFlag").has_value());   // a prefix and nothing else is not a name
        assert(!flagTypeForName("SF").has_value());
        assert(!flagTypeForName("").has_value());
    }

    // S before D before F, and F is always last.
    {
        assert(flagTypeForName("FFlagX").value() == FlagType::Bool);
        assert(flagTypeForName("DFFlagX").value() == FlagType::Bool);
        assert(flagTypeForName("SFFlagX").value() == FlagType::Bool);
        assert(flagTypeForName("SDFFlagX").value() == FlagType::Bool);
        assert(flagTypeForName("SDFIntX").value() == FlagType::Int);
        assert(flagTypeForName("DFStringX").value() == FlagType::String);
        assert(!flagTypeForName("DSFFlagX").has_value());  // D before S
        assert(!flagTypeForName("FSFlagX").has_value());   // F is not first
        assert(!flagTypeForName("SFlagX").has_value());    // no F at all
        assert(!flagTypeForName("FlagX").has_value());     // F eaten, no type word left
    }

    // Only a synchronized name may come from the server.
    {
        assert(!isLocalOnlyFlag("SDFFlagX"));
        assert(isLocalOnlyFlag("DFFlagX"));
        assert(isLocalOnlyFlag("FFlagX"));
    }

    // The flags version is compiled in: no source may replace it.
    {
        assert(flagTypeForName("FStringFlagsVersion").value() == FlagType::String);
        assert(parseFlagJson(R"({"FStringFlagsVersion":"9.9.9"})", false).empty());
        assert(parseFlagJson(R"({"FStringFlagsVersion":"9.9.9"})", true).empty());
        assert(flagString("FStringFlagsVersion") == "2.0.0");
    }

    // A value that contradicts its own name is dropped, so the next source down answers.
    // Keeping it would mean every install guessing the same wrong thing.
    {
        assert(parseFlagJson(R"({"SFFlagA":"yes","SFIntB":4})", false).size() == 1);
        assert(parseFlagJson(R"({"SFIntB":"4"})", false).empty());
        assert(parseFlagJson(R"({"SFStringC":9})", false).empty());
        assert(parseFlagJson(R"({"SFFlagA":1})", false).empty());       // 1 is not true
        assert(parseFlagJson(R"({"SFIntB":4.5})", false).empty());      // not an integer
        // Past int64: nlohmann parses this as UNSIGNED, is_number_integer() is true, and
        // get<int64_t>() wraps it to a large negative. A flag set large and positive
        // would arrive at every install as its negative twin, so it is refused instead.
        assert(parseFlagJson(R"({"SFIntB":9223372036854775808})", false).empty());
        assert(parseFlagJson(R"({"SFIntB":18446744073709551615})", false).empty());
        assert(parseFlagJson(R"({"SFIntB":1e300})", false).empty());
        // The edges themselves still work.
        assert(parseFlagJson(R"({"SFIntB":9223372036854775807})", false)
                   .at("SFIntB").integer == 9223372036854775807LL);
        assert(parseFlagJson(R"({"unknownPrefix":true})", false).empty());
        assert(parseFlagJson(R"({"SFFlagA":{"nested":true}})", false).empty());
        assert(parseFlagJson(R"({"SFFlagA":[true]})", false).empty());
        assert(parseFlagJson(R"({"SFFlagA":null})", false).empty());
    }

    // Not a flat object at all. Every one of these leaves the lower sources alone.
    {
        assert(parseFlagJson("[1,2,3]", false).empty());
        assert(parseFlagJson("\"a string\"", false).empty());
        assert(parseFlagJson("42", false).empty());
        assert(parseFlagJson("null", false).empty());
        assert(parseFlagJson("not json at all", false).empty());
        assert(parseFlagJson("", false).empty());
        assert(parseFlagJson("{", false).empty());
    }

    // The client enforces the local-only list; the server is not trusted to respect it.
    // FFlagDisableUpdater from the server would be a kill switch that cannot be
    // corrected by updating, because updating is what it stops.
    {
        assert(isLocalOnlyFlag("FFlagDisableUpdater"));
        // Read before any route has loaded the flags, so a value from the server could not be seen anyway.
        assert(isLocalOnlyFlag("FFlagBypassSystemRequirements"));
        assert(!isLocalOnlyFlag("SFFlagEnableSeasonalTheme"));
        assert(parseFlagJson(R"({"FFlagDisableUpdater":true})", true).empty());
        assert(parseFlagJson(R"({"FFlagDisableUpdater":true})", false).size() == 1);
        // The rest of a server answer survives its presence.
        assert(parseFlagJson(R"({"FFlagDisableUpdater":true,"SFFlagA":true})", true).size() == 1);
    }

    // Values round-trip with their type.
    {
        const FlagTable t = parseFlagJson(
            R"({"SFFlagA":true,"SFStringB":"2026-10-16","SFIntC":6})", false);
        assert(t.size() == 3);
        assert(t.at("SFFlagA").boolean == true);
        assert(t.at("SFStringB").text == "2026-10-16");
        assert(t.at("SFIntC").integer == 6);
        // A negative and a large integer survive as themselves.
        const FlagTable big = parseFlagJson(R"({"SFIntA":-1,"SFIntB":9007199254740993})", false);
        assert(big.at("SFIntA").integer == -1);
        assert(big.at("SFIntB").integer == 9007199254740993LL);
    }

    // Merge is per key: a key the higher source does not set is left alone, and a key
    // only the higher source has is added -- which is how a flag is introduced ahead of
    // the release that reads it.
    {
        FlagTable base = parseFlagJson(R"({"SFFlagA":false,"SFIntB":1})", false);
        const FlagTable higher = parseFlagJson(R"({"SFFlagA":true,"SFStringC":"x"})", false);
        mergeFlagsOver(base, higher);
        assert(base.size() == 3);
        assert(base.at("SFFlagA").boolean == true);   // overridden
        assert(base.at("SFIntB").integer == 1);       // untouched
        assert(base.at("SFStringC").text == "x");     // introduced
    }

    // Merging an empty table changes nothing -- which is what a failed fetch looks like.
    {
        FlagTable base = parseFlagJson(R"({"SFFlagA":true})", false);
        mergeFlagsOver(base, FlagTable{});
        assert(base.size() == 1 && base.at("SFFlagA").boolean == true);
    }

    printf("flag_table: all tests passed\n");
    return 0;
}
