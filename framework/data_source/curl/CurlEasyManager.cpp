//
// Created by SuperMan on 2021/11/21.
//
#define LOG_TAG "CurlEasyManager"

#include "CurlEasyManager.h"
#include <utils/UrlUtils.h>
#include <utils/frame_work_log.h>
#include <utils/timer.h>

using namespace Cicada;

CurlEasyManager &CurlEasyManager::getInstance()
{
    static CurlEasyManager sCurlEasyManager;
    return sCurlEasyManager;
}

void CurlEasyManager::acquireEasy(const std::string &url, CURL **easyHandler, CURLM **multiHandler)
{
    AF_LOGD("acquireEasy() url = %s", url.c_str());
    URLComponents urlComponents{};
    UrlUtils::parseUrl(urlComponents, url);

    {
        std::lock_guard<std::mutex> lock(easyMutex);
        for (auto &easyItem : mEasyContexts) {
            if (easyItem.busy) {
                continue;
            }

            if (easyItem.host == urlComponents.host && easyItem.proto == urlComponents.proto) {

                easyItem.busy = true;
                *easyHandler = easyItem.easyHandler;
                *multiHandler = easyItem.multiHandler;
                AF_LOGD("acquireEasy() use exits ");
                return;
            }
        }

        AF_LOGD("acquireEasy() not found available easy,create new one ");
        EasyContext easyContext{};
        easyContext.host = urlComponents.host;
        easyContext.proto = urlComponents.proto;
        easyContext.busy = true;
        easyContext.easyHandler = curl_easy_init();
        easyContext.multiHandler = curl_multi_init();
        mEasyContexts.push_back(easyContext);

        *easyHandler = easyContext.easyHandler;
        *multiHandler = easyContext.multiHandler;
    }
}

void CurlEasyManager::releaseEasy(CURL **easyHandler, CURLM **multiHandler)
{
    AF_LOGD("releaseEasy() %p , %p ", *easyHandler, *multiHandler);
    {
        std::lock_guard<std::mutex> lock(easyMutex);
        for (auto &easyItem : mEasyContexts) {
            if (easyItem.easyHandler == *easyHandler && easyItem.multiHandler == *multiHandler) {
                AF_LOGD("releaseEasy() --> in host %s, proto %s ", easyItem.host.c_str(), easyItem.proto.c_str());
                easyItem.busy = false;
                easyItem.idleTimeStamp = af_getsteady_ms();
                curl_easy_reset(easyItem.easyHandler);
                return;
            }
        }
    }
}

CurlEasyManager::CurlEasyManager()
{
    checkIdleThread = std::unique_ptr<afThread>(NEW_AF_THREAD(checkIdleRun));
    checkIdleThread->start();
}

CurlEasyManager::~CurlEasyManager()
{
    stop = true;
    checkIdleThread->stop();
}

void CurlEasyManager::clearEasyContext(bool force)
{
    static int IDLE_TIME_MS = 30 * 1000;
    int64_t timeStamp = af_getsteady_ms();
    {
        std::lock_guard<std::mutex> lock(easyMutex);

        for (auto iter = mEasyContexts.begin(); iter != mEasyContexts.end();) {
            if (force || (!iter->busy && timeStamp - iter->idleTimeStamp > IDLE_TIME_MS)) {
                //clean up the context;
                if (iter->easyHandler && iter->multiHandler) curl_multi_remove_handle(iter->multiHandler, iter->easyHandler);
                if (iter->easyHandler) curl_easy_cleanup(iter->easyHandler);
                if (iter->multiHandler) curl_multi_cleanup(iter->multiHandler);
                AF_LOGD("checkIdleRun() clean old easy %p ,%p", iter->easyHandler, iter->multiHandler);
                iter = mEasyContexts.erase(iter);
                continue;
            }
            ++iter;
        }
    }
}

int CurlEasyManager::checkIdleRun()
{

    clearEasyContext(false);

    {
        std::unique_lock<std::mutex> idleLock(checkIdleMutex);
        checkIdleCondition.wait_for(idleLock, std::chrono::milliseconds(10 * 1000), [this]() -> bool { return stop.load(); });
    }

    if (stop) {
        clearEasyContext(true);
        return -1;
    }

    return 0;
}
