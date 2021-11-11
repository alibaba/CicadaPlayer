//
// Created by 平凯 on 2021/9/8.
//
#define LOG_TAG "CurlMulti"
#include "CurlMulti.h"
#include "CURLConnection2.h"
#include <cassert>
#include <cerrno>
#include <utils/frame_work_log.h>
#include <utils/timer.h>
using namespace Cicada;

class curl_handle_status {
public:
    CURL *curl_handle{nullptr};
    bool eof{false};
    CURLcode status{CURLE_OK};
};

CurlMulti::CurlMulti()
{
    multi_handle = curl_multi_init();
    curl_multi_setopt(multi_handle, CURLMOPT_PIPELINING, CURLPIPE_NOTHING);
    mLoopThread = NEW_AF_THREAD(loop);
}
CurlMulti::~CurlMulti()
{
    curl_multi_wakeup(multi_handle);
    delete mLoopThread;
    {
        std::lock_guard<std::mutex> lockGuard(mListMutex);
        for (auto item : mRemoveList) {
            curl_multi_remove_handle(multi_handle, item->getCurlHandle());
        }
        mRemoveList.clear();
        for (auto item : mDeleteList) {
            item->disableCallBack();
            curl_multi_remove_handle(multi_handle, item->getCurlHandle());
            delete item;
        }
        mDeleteList.clear();
    }
    if (multi_handle) {
        curl_multi_cleanup(multi_handle);
    }
}
int CurlMulti::loop()
{
    CURLMcode mCode;
    int numfds;
    CURLMcode mc = CURLM_OK;
    std::list<CURLConnection2 *> tempList;
    {
        std::lock_guard<std::mutex> lockGuard(mListMutex);
        for (auto item : mRemoveList) {
            tempList.push_back(item);
        }
        mRemoveList.clear();
    }
    for (auto item : tempList) {
        curl_multi_remove_handle(multi_handle, item->getCurlHandle());
    }
    tempList.clear();
    {
        std::lock_guard<std::mutex> lockGuard(mListMutex);
        for (auto item : mAddList) {
            curl_multi_add_handle(multi_handle, item->getCurlHandle());
        }
        mAddList.clear();
    }
    {
        std::lock_guard<std::mutex> lockGuard(mListMutex);
        for (auto item : mDeleteList) {
            item->disableCallBack();
            tempList.push_back(item);
        }
        mDeleteList.clear();
    }
    for (auto item : tempList) {
        curl_multi_remove_handle(multi_handle, item->getCurlHandle());
        delete item;
    }


    do {
        mCode = curl_multi_perform(multi_handle, &mStillRunning);
    } while (mCode == CURLM_CALL_MULTI_PERFORM);

    int msgs;
    CURLMsg *msg;
    curl_handle_status status{};
    while ((msg = curl_multi_info_read(multi_handle, &msgs))) {
        CURLConnection2 *curl_connection = nullptr;
        curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &curl_connection);
        assert(curl_connection != nullptr);
        if (msg->msg == CURLMSG_DONE) {
            if (msg->data.result == CURLE_OK) {
                status.eof = true;
            }
            status.status = msg->data.result;
        } else {
            if (!mStillRunning && msg->data.result == CURLE_OK) {
                // we assume eof
                assert(0);
                status.eof = true;
                AF_LOGW("assume a abnormal eos\n");
                status.status = CURLE_OK;
            }
        }
        if (status.eof || status.status != CURLE_OK) {
            curl_connection->onStatus(status.eof, status.status);
            curl_multi_remove_handle(multi_handle, curl_connection->getCurlHandle());
        }
    }

    if (mStillRunning) {
        mc = curl_multi_poll(multi_handle, nullptr, 0, 2, &numfds);
    }

    if (!mStillRunning) {
        af_msleep(10);
    }
    if (mc) {
        AF_LOGE("curl_multi_poll error %d\n", mc);
    }
    return 0;
}
CURLMcode CurlMulti::addHandle(CURLConnection2 *curl_connection)
{
    if (mLoopThread->getStatus() == afThread::THREAD_STATUS_IDLE) {
        mLoopThread->start();
    }
    std::lock_guard<std::mutex> lockGuard(mListMutex);
    for (auto &item : mRemoveList) {
        if (item == curl_connection) {
            mRemoveList.remove(item);
            break;
        }
    }
    curl_easy_setopt(curl_connection->getCurlHandle(), CURLOPT_PRIVATE, curl_connection);
    mAddList.push_back(curl_connection);
    curl_multi_wakeup(multi_handle);
    return CURLM_OK;
    //   return curl_multi_add_handle(multi_handle, curl_handle);
}
CURLMcode CurlMulti::removeHandle(CURLConnection2 *curl_connection)
{
    std::lock_guard<std::mutex> lockGuard(mListMutex);
    for (auto &item : mAddList) {
        if (item == curl_connection) {
            mAddList.remove(item);
            break;
        }
    }
    mRemoveList.push_back(curl_connection);
    curl_multi_wakeup(multi_handle);
    return CURLM_OK;
    //    return curl_multi_remove_handle(multi_handle, curl_handle);
}
void CurlMulti::deleteHandle(CURLConnection2 *curl_connection)
{
    std::lock_guard<std::mutex> lockGuard(mListMutex);
    for (auto &item : mAddList) {
        if (item == curl_connection) {
            mAddList.remove(item);
            break;
        }
    }
    mDeleteList.push_back(curl_connection);
    curl_multi_wakeup(multi_handle);
}
int CurlMulti::poll(int time_ms)
{
    //    int numfds;
    //    std::lock_guard<std::mutex> lockGuard(mMutex);
    //    CURLMcode mc = curl_multi_poll(multi_handle, nullptr, 0, time_ms, &numfds);
    //
    //    return mc;
    af_msleep(time_ms);
    return 0;
}
int CurlMulti::wakeUp()
{
    curl_multi_wakeup(multi_handle);
    return 0;
}
