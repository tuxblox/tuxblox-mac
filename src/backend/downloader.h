// TuxBlox Mac - Linux launcher for Roblox's macOS Player
// Copyright (C) 2026 TuxBlox Developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <string>

namespace tuxblox {

enum class DownloadResult { Ok, Cancelled, Failed };

struct DownloadOutcome {
    DownloadResult result = DownloadResult::Failed;
    std::string errorMessage;
};

using DownloadProgressFn = std::function<void(uint64_t, uint64_t)>;

DownloadOutcome downloadFile(const std::string& url,
                              const std::string& destPath,
                              const DownloadProgressFn& onProgress,
                              const std::atomic<bool>* cancel);

} // namespace tuxblox
