//
// Created by SuperMan on 2021/11/21.
//

#ifndef CICADAMEDIA_CURLEASYMANAGER_H
#define CICADAMEDIA_CURLEASYMANAGER_H

#include <curl/curl.h>
#include <mutex>
#include <string>
#include <utils/afThread.h>
#include <vector>

class CurlEasyManager {

    class EasyContext {
    public:
        std::string host{};
        std::string proto{};
        bool busy{false};
        CURL *easyHandler{nullptr};
        CURLM *multiHandler{nullptr};
        int64_t idleTimeStamp{0};
    };

public:
    static CurlEasyManager &getInstance();

    /**
     *
     * @param url
     * @param easyHandler must not be nullptr
     * @param multiHandler  can be nullptr
     */
    void acquireEasy(const std::string &url, CURL **easyHandler, CURLM **multiHandler);

    /**
     *
     * @param easyHandler must not be nullptr
     * @param multiHandler can be nullptr
     */
    void releaseEasy(CURL **easyHandler, CURLM **multiHandler);

private:
    CurlEasyManager();
    ~CurlEasyManager();

private:
    int checkIdleRun();

    void clearEasyContext(bool force);

private:
    std::mutex easyMutex{};
    std::vector<EasyContext> mEasyContexts{};

    std::atomic_bool stop{false};
    std::mutex checkIdleMutex{};
    std::condition_variable checkIdleCondition{};
    std::unique_ptr<afThread> checkIdleThread{};
};


#endif//CICADAMEDIA_CURLEASYMANAGER_H
