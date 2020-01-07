//
//  CURLShareInstance
//  source
//
//  Created by huang_jiafa on 2019/04/02.
//  Copyright (c) 2019 Aliyun. All rights reserved.
//

#include "CURLShareInstance.h"
#include <mutex>
#include <cassert>

extern "C" {
#include <libavformat/avformat.h>
};

namespace Cicada {
    CURLShareInstance CURLShareInstance::sInstance{};

    CURLShareInstance::CURLShareInstance()
    {
        mShareWithDNS = unique_ptr<curlShare>(new curlShare());
        mShare = unique_ptr<curlShare>(new curlShare(CURL_LOCK_DATA_SSL_SESSION));
    }

    CURLShareInstance::~CURLShareInstance() = default;

    CURLShareInstance *CURLShareInstance::Instance()
    {
        return &sInstance;
    }


    curl_slist *CURLShareInstance::getHosts(const string &url, CURLSH **sh)
    {
        std::unique_lock<std::mutex> uMutex(globalSettings::getSetting()->getMutex());
        const globalSettings::type_resolve &resolve = globalSettings::getSetting()->getResolve();
        curl_slist *host = nullptr;
        char proto[256];
        char hostname[256];
        int port = 0;
        av_url_split(proto, sizeof(proto), nullptr, 0, hostname, sizeof(hostname), &port,
                     nullptr, 0, url.c_str());

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

}// namespace Cicada

