//
// Created by 平凯 on 2021/9/8.
//

#ifndef CICADAMEDIA_CURLMULTI_H
#define CICADAMEDIA_CURLMULTI_H

#include <curl/curl.h>
#include <list>
#include <mutex>
#include <utils/afThread.h>

#define SOCKET_ERROR (-1)

namespace Cicada {
    class CURLConnection2;
    class CurlMulti {
    public:
        CurlMulti();
        ~CurlMulti();

        CURLMcode addHandle(CURL *curl_handle);
        CURLcode performHandle(CURL *curl_handle, bool &eof, CURLMcode &mCode);
        CURLMcode removeHandle(CURL *curl_handle);
        void deleteHandle(CURLConnection2 *curl_connection);
        int poll(int time_ms);
        int wakeUp();

    private:
        int loop();

    private:
        CURLM *multi_handle{nullptr};
        std::mutex mMutex;
        afThread *mLoopThread{};
        int mStillRunning{0};

        std::mutex mListMutex;
        std::list<CURL *> mAddList;
        std::list<CURL *> mRemoveList;
        std::list<CURLConnection2 *> mDeleteList;

        class curl_handle_status {
        public:
            CURL *curl_handle{nullptr};
            bool eof{false};
            CURLcode status{CURLE_OK};
        };
        std::mutex mStatusListMutex;
        std::list<curl_handle_status> mStatusList;
    };

    class CurlMultiManager {
    public:
        static CurlMulti *getCurlMulti()
        {
            static CurlMulti instance;
            return &instance;
        }
    };
}// namespace Cicada


#endif//CICADAMEDIA_CURLMULTI_H
