// TuxBlox Mac - Linux launcher for Roblox's macOS Player
// Copyright (C) 2026 TuxBlox Developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#pragma once
#include <atomic>
#include <string>

namespace tuxblox {

// "live" unless the caller named another channel.
std::string normalizeChannel(const std::string& channel);

// The `MacPlayer` client-version document. Throws std::runtime_error on any
// transport or HTTP error, and on a body that is not the expected object.
std::string fetchClientVersionJson(const std::string& binaryType, const std::string& channel,
                                    const std::atomic<bool>* cancel = nullptr);

std::string parseClientVersionHash(const std::string& json);
std::string parseClientVersionNumber(const std::string& json);

} // namespace tuxblox
