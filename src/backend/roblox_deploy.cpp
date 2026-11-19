// TuxBlox Mac - Linux launcher for Roblox's macOS Player
// Copyright (C) 2026 TuxBlox Developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "roblox_deploy.h"

#include "json.hpp"

#include <curl/curl.h>

#include <algorithm>
#include <cctype>
#include <mutex>
#include <stdexcept>

namespace tuxblox {
namespace {

constexpr const char *kDefaultChannel = "live";
constexpr const char *kDefaultChannelAlias = "production";
std::once_flag gCurlInit;

size_t appendBody(char *pData, size_t size, size_t count, void *pUser) {
    auto *pBody = static_cast<std::string *>(pUser);
    pBody->append(pData, size * count);
    return size * count;
}

std::string httpGet(const std::string& url, const std::atomic<bool>* cancel) {
    std::call_once(gCurlInit, [] { curl_global_init(CURL_GLOBAL_DEFAULT); });

    CURL *pCurl = curl_easy_init();
    if (pCurl == nullptr) throw std::runtime_error("cannot start an HTTP request");

    std::string body;
    curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(pCurl, CURLOPT_MAXREDIRS, 10L);
    curl_easy_setopt(pCurl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 15L);
    curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(pCurl, CURLOPT_USERAGENT, "TuxBloxMac/0.1");
    curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, appendBody);
    curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &body);
    if (cancel != nullptr) {
        // A short poll of the caller's cancellation flag is enough here: this
        // request is only the small version document, not the client ZIP.
        curl_easy_setopt(pCurl, CURLOPT_XFERINFOFUNCTION,
                         +[](void *pUser, curl_off_t, curl_off_t, curl_off_t, curl_off_t) {
                             return static_cast<const std::atomic<bool> *>(pUser)->load() ? 1 : 0;
                         });
        curl_easy_setopt(pCurl, CURLOPT_XFERINFODATA, cancel);
        curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, 0L);
    }

    const CURLcode code = curl_easy_perform(pCurl);
    curl_easy_cleanup(pCurl);
    if (code != CURLE_OK) {
        throw std::runtime_error(std::string("HTTP request failed: ") + curl_easy_strerror(code));
    }
    return body;
}

std::string jsonField(const std::string& json, const char *pName) {
    try {
        nlohmann::json parsed = nlohmann::json::parse(json);
        return parsed.at(pName).get<std::string>();
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error(std::string("unexpected version response: ") + e.what());
    }
}

} // namespace

std::string normalizeChannel(const std::string& channel) {
    const size_t begin = channel.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) return kDefaultChannel;
    const size_t end = channel.find_last_not_of(" \t\r\n");
    std::string name = channel.substr(begin, end - begin + 1);
    std::transform(name.begin(), name.end(), name.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (name == kDefaultChannelAlias) return kDefaultChannel;
    return name;
}

std::string fetchClientVersionJson(const std::string& binaryType, const std::string& channel,
                                    const std::atomic<bool>* cancel) {
    std::string url = "https://clientsettings.roblox.com/v2/client-version/" + binaryType;
    const std::string name = normalizeChannel(channel);
    if (name != kDefaultChannel) url += "/channel/" + name;
    return httpGet(url, cancel);
}

std::string parseClientVersionHash(const std::string& json) {
    return jsonField(json, "clientVersionUpload");
}

std::string parseClientVersionNumber(const std::string& json) {
    return jsonField(json, "version");
}

} // namespace tuxblox
