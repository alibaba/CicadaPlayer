//
// Created by moqi on 2019/11/20.
//

#ifndef CICADAPLAYERSDK_CURLSHARE_H
#define CICADAPLAYERSDK_CURLSHARE_H

#include <cstdint>
#include <curl/curl.h>
#include <mutex>

namespace Cicada {
    class curlShare {
    public:
        static const uint64_t SHARED_SSL_SESSION = 1 << 0;
        static const uint64_t SHARED_DNS = 1 << 1;

        explicit curlShare(uint64_t flags = SHARED_SSL_SESSION | SHARED_DNS);

        ~curlShare();

        explicit operator CURLSH *() const ;

    private:
        static void lockData(CURL *handle, curl_lock_data data, curl_lock_access access, void *useptr);

        static void unlockData(CURL *handle, curl_lock_data data, void *useptr);

    private:
        CURLSH *mShare = nullptr;
        std::mutex mutexes_[CURL_LOCK_DATA_LAST + 1];

    };
}


#endif //CICADAPLAYERSDK_CURLSHARE_H
