// TuxBlox Mac - Linux launcher for Roblox's macOS Player
// Copyright (C) 2026 TuxBlox Developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "downloader.h"

#include <curl/curl.h>

#include <cstdio>
#include <filesystem>
#include <mutex>
#include <system_error>

namespace fs = std::filesystem;

namespace tuxblox {
namespace {

std::once_flag gCurlInit;

struct Progress {
    const DownloadProgressFn *pOnProgress = nullptr;
    const std::atomic<bool> *pCancel = nullptr;
    uint64_t lastNow = 0;
};

size_t writeBody(char *pData, size_t size, size_t count, void *pUser) {
    return std::fwrite(pData, size, count, static_cast<FILE *>(pUser));
}

int progressCallback(void *pUser, curl_off_t dltotal, curl_off_t dlnow, curl_off_t, curl_off_t) {
    auto *pProgress = static_cast<Progress *>(pUser);
    if (pProgress->pCancel != nullptr && pProgress->pCancel->load()) return 1;
    if (pProgress->pOnProgress != nullptr && dlnow != static_cast<curl_off_t>(pProgress->lastNow)) {
        pProgress->lastNow = static_cast<uint64_t>(dlnow);
        (*pProgress->pOnProgress)(static_cast<uint64_t>(dlnow), static_cast<uint64_t>(dltotal));
    }
    return 0;
}

} // namespace

DownloadOutcome downloadFile(const std::string& url, const std::string& destPath,
                              const DownloadProgressFn& onProgress,
                              const std::atomic<bool>* cancel) {
    std::call_once(gCurlInit, [] { curl_global_init(CURL_GLOBAL_DEFAULT); });

    std::error_code ec;
    const fs::path dest(destPath);
    if (dest.has_parent_path()) fs::create_directories(dest.parent_path(), ec);

    FILE *pFile = std::fopen(destPath.c_str(), "wb");
    if (pFile == nullptr) return {DownloadResult::Failed, "cannot open the download destination"};

    CURL *pCurl = curl_easy_init();
    if (pCurl == nullptr) {
        std::fclose(pFile);
        return {DownloadResult::Failed, "cannot start the download"};
    }

    Progress progress;
    progress.pOnProgress = &onProgress;
    progress.pCancel = cancel;

    curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(pCurl, CURLOPT_MAXREDIRS, 10L);
    curl_easy_setopt(pCurl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 20L);
    curl_easy_setopt(pCurl, CURLOPT_USERAGENT, "TuxBloxMac/0.1");
    curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, writeBody);
    curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, pFile);
    curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(pCurl, CURLOPT_XFERINFOFUNCTION, progressCallback);
    curl_easy_setopt(pCurl, CURLOPT_XFERINFODATA, &progress);

    const CURLcode code = curl_easy_perform(pCurl);
    std::fclose(pFile);
    curl_easy_cleanup(pCurl);

    if (code == CURLE_ABORTED_BY_CALLBACK) {
        fs::remove(dest, ec);
        return {DownloadResult::Cancelled, "download cancelled"};
    }
    if (code != CURLE_OK) {
        fs::remove(dest, ec);
        return {DownloadResult::Failed, curl_easy_strerror(code)};
    }
    if (!fs::is_regular_file(dest, ec) || ec || fs::file_size(dest, ec) == 0) {
        fs::remove(dest, ec);
        return {DownloadResult::Failed, "the server returned an empty download"};
    }
    return {DownloadResult::Ok, std::string()};
}

} // namespace tuxblox
