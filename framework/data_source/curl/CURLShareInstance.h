//
//  CURLShareInstance
//  source
//
//  Created by huang_jiafa on 2019/04/02.
//  Copyright (c) 2019 Aliyun. All rights reserved.
//

#ifndef CURLShareInstance_H
#define CURLShareInstance_H

#include <curl/curl.h>
#include <mutex>
#include <utils/globalSettings.h>
#include "curlShare.h"

namespace Cicada{

    class CURLShareInstance {
    public:
        static CURLShareInstance *Instance();

        curl_slist *getHosts(const string &url, CURLSH **sh);
    private:
        CURLShareInstance();

        ~CURLShareInstance();

    private:
        static CURLShareInstance sInstance;
        std::unique_ptr<curlShare> mShareWithDNS{};
        std::unique_ptr<curlShare> mShare{};
    };

}// namespace Cicada
#endif // ANALYTICS_COLLECTOR_SAAS_FACTORY_H
