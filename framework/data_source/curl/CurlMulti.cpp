//
// Created by 平凯 on 2021/9/8.
//
#define LOG_TAG "CurlMulti"
#include "CurlMulti.h"
#include <cassert>
#include <cerrno>
#include <utils/frame_work_log.h>
#include <utils/timer.h>
using namespace Cicada;

CurlMulti::CurlMulti()
{
    multi_handle = curl_multi_init();
    curl_multi_setopt(multi_handle, CURLMOPT_PIPELINING, CURLPIPE_NOTHING);
    mLoopThread = NEW_AF_THREAD(loop);
}
CurlMulti::~CurlMulti()
{
    delete mLoopThread;
    if (multi_handle) {
        curl_multi_cleanup(multi_handle);
    }
}
int CurlMulti::loop()
{
    CURLMcode mCode;
    int numfds;
    CURLMcode mc = CURLM_OK;
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        do {
            mCode = curl_multi_perform(multi_handle, &mStillRunning);
        } while (mCode == CURLM_CALL_MULTI_PERFORM);

        int msgs;
        CURLMsg *msg;
        {
            std::lock_guard<std::mutex> lock_guard(mStatusListMutex);
            mStatusList.clear();

            curl_handle_status status{};

            while ((msg = curl_multi_info_read(multi_handle, &msgs))) {
                status.curl_handle = msg->easy_handle;
                CURL *handler = msg->easy_handle;
                status.status = msg->data.result;
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
                mStatusList.push_back(status);
            }
        }

        if (mStillRunning) {
            mc = curl_multi_poll(multi_handle, nullptr, 0, 10, &numfds);
        }
    }
    if (!mStillRunning) {
        af_msleep(10);
    }
    if (mc) {
        AF_LOGE("curl_multi_poll error %d\n", mc);
    }
    return 0;
}
CURLMcode CurlMulti::addHandle(CURL *curl_handle)
{
    if (mLoopThread->getStatus() == afThread::THREAD_STATUS_IDLE) {
        mLoopThread->start();
    }
    std::lock_guard<std::mutex> lockGuard(mMutex);
    return curl_multi_add_handle(multi_handle, curl_handle);
}
CURLcode CurlMulti::performHandle(CURL *curl_handle, bool &eof, CURLMcode &mCode)
{
    mCode = CURLM_OK;
    std::lock_guard<std::mutex> lock_guard(mStatusListMutex);
    for (auto &item : mStatusList) {
        if (item.curl_handle == curl_handle) {
            eof = item.eof;
            return item.status;
        }
    }
    eof = false;
    return CURLE_OK;
#if 0
    //    int still_running;
    //    do {
    //        mCode = curl_multi_perform(multi_handle, &still_running);
    //    } while (mCode == CURLM_CALL_MULTI_PERFORM);
    //
    //    if (mCode != CURLM_OK) {
    //        // TODO: return what??
    //        return CURLE_OK;
    //    }

    int msgs;
    //    CURLcode CURLResult = CURLE_OK;
    CURLMsg *msg;
    eof = false;
    mCode = CURLM_OK;

    while ((msg = curl_multi_info_read(multi_handle, &msgs))) {
        CURL *handler = msg->easy_handle;
        if (handler != curl_handle) {
            continue;
        }

        if (msg->msg == CURLMSG_DONE) {
            if (msg->data.result == CURLE_OK) {
                eof = true;
            }
            return msg->data.result;
        } else {
            if (!mStillRunning && msg->data.result == CURLE_OK) {
                // we assume eof
                assert(0);
                eof = true;
                AF_LOGW("assume a abnormal eos\n");
                return CURLE_OK;
            }
        }
        break;
    }

    // TODO: return what??
    return CURLE_OK;
#endif
}
CURLMcode CurlMulti::removeHandle(CURL *curl_handle)
{
    std::lock_guard<std::mutex> lockGuard(mMutex);
    return curl_multi_remove_handle(multi_handle, curl_handle);
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
