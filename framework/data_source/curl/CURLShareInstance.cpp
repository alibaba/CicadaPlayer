//
//  CURLShareInstance
//  source
//
//  Created by huang_jiafa on 2019/04/02.
//  Copyright (c) 2019 Aliyun. All rights reserved.
//

#include "CURLShareInstance.h"
#include <cassert>
#include <mutex>

extern "C" {
#include <libavformat/avformat.h>
};

using namespace Cicada;

static curl_sslbackend getCurlSslBackend()
{
    const curl_ssl_backend **list;
    CURLsslset result = curl_global_sslset((curl_sslbackend) -1, nullptr, &list);
    assert(result == CURLSSLSET_UNKNOWN_BACKEND);

    // we only build one ssl backend
    if (list[0]) {
        return list[0]->id;
    }

    return CURLSSLBACKEND_NONE;
}

CURLShareInstance::CURLShareInstance()
{
    mSslbackend = getCurlSslBackend();
    curl_global_init(CURL_GLOBAL_DEFAULT);
    mShareWithDNS = unique_ptr<curlShare>(new curlShare());
    mShare = unique_ptr<curlShare>(new curlShare(CURL_LOCK_DATA_SSL_SESSION));
}

CURLShareInstance::~CURLShareInstance()
{
    curl_global_cleanup();
}

CURLShareInstance *CURLShareInstance::Instance()
{
    static CURLShareInstance sInstance;
    return &sInstance;
}


curl_slist *CURLShareInstance::getHosts(const string &url, CURLSH **sh)
{
    std::unique_lock<std::mutex> uMutex(globalSettings::getSetting().getMutex());
    const globalSettings::type_resolve &resolve = globalSettings::getSetting().getResolve();
    curl_slist *host = nullptr;
    char proto[256];
    char hostname[256];
    int port = 0;
    av_url_split(proto, sizeof(proto), nullptr, 0, hostname, sizeof(hostname), &port, nullptr, 0, url.c_str());

    if (port <= 0) {
        if (strcmp(proto, "http") == 0) {
            port = 80;
        } else if (strcmp(proto, "https") == 0) {
            port = 443;
        }
    }

    assert(port > 0);
    string hostName = hostname;
    hostName += ":" + to_string(port);
    auto resolveItem = resolve.find(hostName);
    *sh = (CURLSH *) (*mShare);

    if (resolveItem == resolve.end() || (*resolveItem).second.empty()) {
        return host;
    }

    string content = hostName + ":";
    bool first = true;

    for (const auto &ip : (*resolveItem).second) {
        if (!first) {
            content += ",";
        }

        content += ip;
        first = false;
    }

    assert(!content.empty());
    host = curl_slist_append(nullptr, content.c_str());
    *sh = (CURLSH *) (*mShareWithDNS);
    return host;
}

curl_sslbackend CURLShareInstance::getSslbakcend()
{
    return mSslbackend;
}