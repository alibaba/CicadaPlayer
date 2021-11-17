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

        CURLMcode addHandle(CURLConnection2 *curl_connection);
        CURLMcode removeHandle(CURLConnection2 *curl_connection);
        void deleteHandle(CURLConnection2 *curl_connection);
        void resumeHandle(CURLConnection2 *curl_connection);
        int poll(int time_ms);

    private:
        int loop();
        void applyPending();

    private:
        CURLM *multi_handle{nullptr};
        std::mutex mMutex;
        afThread *mLoopThread{};
        int mStillRunning{0};

        std::mutex mListMutex;
        std::list<CURLConnection2 *> mAddList;
        std::list<CURLConnection2 *> mRemoveList;
        std::list<CURLConnection2 *> mDeleteList;
        std::list<CURLConnection2 *> mResumeList;
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
