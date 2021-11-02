//
// Created by moqi on 2020/2/12.
//
#define LOG_TAG "CURLConnection2"

#include "CURLConnection2.h"
#include "CURLShareInstance.h"
#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <utils/errors/framework_error.h>
#include <utils/frame_work_log.h>
#include <utils/timer.h>

using namespace std;
using namespace Cicada;
#define MAX_RESPONSE_SIZE 1024

#define RINGBUFFER_SIZE (1024 * 256)
#define RINGBUFFER_BACK_SIZE (1024 * 512)

#define MIN_SO_RCVBUF_SIZE (1024 * 64)
#define READ_BUFFER_SIZE CURL_MAX_WRITE_SIZE//(1024 * 64)

#define SOCKET_ERROR (-1)

#define FITS_INT(a) (((a) <= INT_MAX) && ((a) >= INT_MIN))

#define FALSE 0L
#define TRUE 1L

#define CURL_LOGD(...)                                                                                                                     \
    do {                                                                                                                                   \
        if (mPConfig == nullptr || mPConfig->enableLog) __log_print(AF_LOG_LEVEL_DEBUG, LOG_TAG, __VA_ARGS__);                             \
    } while (0)

static int getErrorCode(const CURLcode &CURLResult)
{
    if (CURLResult == CURLE_COULDNT_RESOLVE_HOST) {
        return gen_framework_errno(error_class_network, network_errno_resolve);
    }

    if (CURLResult == CURLE_COULDNT_CONNECT) return gen_framework_errno(error_class_network, network_errno_could_not_connect);

    if (CURLResult == CURLE_OPERATION_TIMEDOUT) {
        return gen_framework_errno(error_class_network, network_errno_connect_timeout);
    }

    return FRAMEWORK_ERR(EIO);
}

Cicada::CURLConnection2::CURLConnection2(Cicada::IDataSource::SourceConfig *pConfig, CurlMulti *multi, CURLConnectionListener *listener)
{
    mHttp_handle = curl_easy_init();
    pRbuf = RingBufferCreate(RINGBUFFER_SIZE + RINGBUFFER_BACK_SIZE);
    RingBufferSetBackSize(pRbuf, RINGBUFFER_BACK_SIZE);
    m_bFirstLoop = 1;
    mPConfig = pConfig;
    mMulti = multi;
    mListener = listener;

    if (mPConfig) {
        so_rcv_size = pConfig->so_rcv_size;
        string &http_proxy = pConfig->http_proxy;

        if (!http_proxy.empty()) {
            CURL_LOGD("http_proxy is %s\n", http_proxy.c_str());

            if (http_proxy == "never") {
                curl_easy_setopt(mHttp_handle, CURLOPT_PROXY, NULL);
            } else {
                curl_easy_setopt(mHttp_handle, CURLOPT_PROXY, http_proxy.c_str());
            }
        }

        string &refer = pConfig->refer;

        if (!refer.empty()) {
            CURL_LOGD("refer is %s\n", refer.c_str());
            curl_easy_setopt(mHttp_handle, CURLOPT_REFERER, refer.c_str());
        }

        string &userAgent = pConfig->userAgent;

        if (!userAgent.empty()) {
            CURL_LOGD("userAgent is %s\n", userAgent.c_str());
            curl_easy_setopt(mHttp_handle, CURLOPT_USERAGENT, userAgent.c_str());
        }

        if (pConfig->low_speed_limit && pConfig->low_speed_time_ms) {
            CURL_LOGD("set low_speed_limit to %d\n", pConfig->low_speed_limit);
            CURL_LOGD("set low_speed_time to %d(ms)\n", pConfig->low_speed_time_ms);
            curl_easy_setopt(mHttp_handle, CURLOPT_LOW_SPEED_LIMIT, (long) pConfig->low_speed_limit);
            curl_easy_setopt(mHttp_handle, CURLOPT_LOW_SPEED_TIME, (long) pConfig->low_speed_time_ms / 1000);
        }

        if (pConfig->connect_time_out_ms > 0) {
            CURL_LOGD("set connect_time to %d(ms)\n", pConfig->connect_time_out_ms);
            curl_easy_setopt(mHttp_handle, CURLOPT_CONNECTTIMEOUT, (long) pConfig->connect_time_out_ms / 1000);
        }

        switch (pConfig->resolveType) {
            case IDataSource::SourceConfig::IpResolveV4:
                curl_easy_setopt(mHttp_handle, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
                break;
            case IDataSource::SourceConfig::IpResolveV6:
                curl_easy_setopt(mHttp_handle, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V6);
                break;

            default:
                break;
        }
    }

    // TODO: move to conncet
    curl_easy_setopt(mHttp_handle, CURLOPT_SOCKOPTFUNCTION, sockopt_callback);
    curl_easy_setopt(mHttp_handle, CURLOPT_SOCKOPTDATA, this);
    esayHandle_set_common_opt();
}

void CURLConnection2::setSSLBackEnd(curl_sslbackend sslbackend)
{
    //For https ignore CA certificates
    curl_easy_setopt(mHttp_handle, CURLOPT_SSL_VERIFYPEER, FALSE);

    // openssl not verify host, otherwise will return CURLE_PEER_FAILED_VERIFICATION
    if (sslbackend == CURLSSLBACKEND_OPENSSL) {
        curl_easy_setopt(mHttp_handle, CURLOPT_SSL_VERIFYHOST, FALSE);
    }
}

Cicada::CURLConnection2::~CURLConnection2()
{
    if (mHttp_handle) {
        curl_easy_cleanup(mHttp_handle);
    }

    if (pRbuf) {
        RingBufferDestroy(pRbuf);
    }
    if (response) {
        free(response);
    }

    if (reSolveList) {
        curl_slist_free_all(reSolveList);
    }
}

void Cicada::CURLConnection2::reset()
{
    RingBufferClear(pRbuf);
    responseSize = 0;
    mDNSResolved = false;

    if (response) {
        response[0] = 0;
    }
}
int CURLConnection2::xferinfo(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    if (!clientp) {
        return 0;
    }
    auto *pHandle = (CURLConnection2 *) clientp;
    if (pHandle->mPaused && RingBuffergetMaxWriteSize(pHandle->pRbuf) > READ_BUFFER_SIZE) {
        pHandle->mPaused = false;
        curl_easy_pause(pHandle->mHttp_handle, CURLPAUSE_CONT);
    }
    return 0;
}

size_t Cicada::CURLConnection2::write_callback(char *buffer, size_t size, size_t nitems, void *userp)
{
    if (!userp) {
        return CURL_WRITEFUNC_PAUSE;
    }
    auto *pHandle = (CURLConnection2 *) userp;
    //    std::lock_guard<std::mutex>lockGuard(pHandle->mCurlCbMutex);
    assert(!pHandle->mPaused);
    auto amount = (uint32_t) (size * nitems);

    if (RingBuffergetMaxWriteSize(pHandle->pRbuf) < amount) {
        pHandle->mPaused = true;
        AF_LOGD("write_callback %p paused\n", pHandle);
        return CURL_WRITEFUNC_PAUSE;
    }

    if (RingBufferWriteData(pHandle->pRbuf, buffer, amount) != amount) {
        AF_LOGE("write ring buffer error\n");
        assert(0);
    }
    if (pHandle->mPConfig && pHandle->mPConfig->listener) {
        // TODO: get file type form content type
        pHandle->mPConfig->listener->onNetWorkInPut(amount, IDataSource::Listener::bitStreamTypeMedia);
    }
    return amount;
}

void Cicada::CURLConnection2::debugHeader(bool in, char *data, size_t size)
{
    string *header = &outHeader;

    if (in) {
        header = &inHeader;
    }

    *header += string(data, size);

    if (header->length() < 4) {
        return;
    }

    if (header->compare(header->length() - 4, 4, "\r\n\r\n") == 0) {
        CURL_LOGD("<= %s header \n%s", in ? "Recv" : "Send", header->c_str());
        *header = "";
    }
}

size_t Cicada::CURLConnection2::write_response(void *ptr, size_t size, size_t nmemb, void *data)
{
    if (!data) {
        return size * nmemb;
    }
    auto *pHandle = (CURLConnection2 *) data;

    if (pHandle->response == nullptr) {
        pHandle->response = (char *) malloc(MAX_RESPONSE_SIZE);
        memset(pHandle->response, 0, MAX_RESPONSE_SIZE);
        pHandle->responseSize = 0;
    }

    if (pHandle->responseSize + size * nmemb < MAX_RESPONSE_SIZE) {
        memcpy(pHandle->response + pHandle->responseSize, ptr, size * nmemb);
        pHandle->responseSize += size * nmemb;
    }

    return size * nmemb;
}

int Cicada::CURLConnection2::sockopt_callback(void *clientp, curl_socket_t curlfd, curlsocktype purpose)
{
    auto *connect = (Cicada::CURLConnection2 *) clientp;
    if (purpose == CURLSOCKTYPE_IPCXN) {
        int rcv_buffer = connect->so_rcv_size;
        if (rcv_buffer > 0) {
            setsockopt(curlfd, SOL_SOCKET, SO_RCVBUF, (const char *) &rcv_buffer, sizeof(rcv_buffer));
        }

        if (connect->mListener) {
            connect->mListener->onDNSResolved();
        }
        connect->mDNSResolved = true;
    }

    /* This return code was added in libcurl 7.21.5 */
    return 0;//CURL_SOCKOPT_ALREADY_CONNECTED;
}

void Cicada::CURLConnection2::setSource(const string &location, struct curl_slist *headerList)
{
    if (headerList) {
        curl_easy_setopt(mHttp_handle, CURLOPT_HTTPHEADER, headerList);
    } else {
        curl_easy_setopt(mHttp_handle, CURLOPT_HTTPHEADER, NULL);
    }

    uri = location;
    curl_easy_setopt(mHttp_handle, CURLOPT_URL, uri.c_str());
    CURLSH *sh = nullptr;

    if (reSolveList) {
        curl_slist_free_all(reSolveList);
    }

    reSolveList = CURLShareInstance::Instance()->getHosts(uri, &sh);
    assert(sh != nullptr);
    curl_easy_setopt(mHttp_handle, CURLOPT_SHARE, sh);

    if (reSolveList != nullptr) {
        curl_easy_setopt(mHttp_handle, CURLOPT_RESOLVE, reSolveList);
    }
}
void CURLConnection2::setPost(bool post, int64_t size, const uint8_t *data)
{
    if (post) {
        curl_easy_setopt(mHttp_handle, CURLOPT_POST, 1);
        curl_easy_setopt(mHttp_handle, CURLOPT_POSTFIELDSIZE, (long) size);
        curl_easy_setopt(mHttp_handle, CURLOPT_POSTFIELDS, data);

    } else {
        curl_easy_setopt(mHttp_handle, CURLOPT_POST, 0);
    }
}

int CURLConnection2::my_trace(CURL *handle, curl_infotype type, char *data, size_t size, void *userp)
{
    if (!userp) {
        return 0;
    }
    const char *text;
    //  int num = hnd2num(handle);
    auto *pHandle = (CURLConnection2 *) userp;
    (void) pHandle; /* prevent compiler warning */

    switch (type) {
        case CURLINFO_TEXT:
            if (pHandle->mPConfig->enableLog) {
                AF_LOGD("== Info: %s", data);
            }
        default: /* in case a new one is introduced to shock us */
            return 0;

        case CURLINFO_HEADER_OUT:
            pHandle->debugHeader(false, data, size);
            return 0;

        case CURLINFO_DATA_OUT:
            text = "=> Send data";
            break;

        case CURLINFO_SSL_DATA_OUT:
            text = "=> Send SSL data";
            break;

        case CURLINFO_HEADER_IN:
            pHandle->debugHeader(true, data, size);
            return 0;

        case CURLINFO_DATA_IN:
            text = "<= Recv data";
            break;

        case CURLINFO_SSL_DATA_IN:
            text = "<= Recv SSL data";
            break;
    }

    //  CURL_LOGD("%s\n", text);
    return 0;
}

int Cicada::CURLConnection2::esayHandle_set_common_opt()
{
    if (log_get_level() >= AF_LOG_LEVEL_TRACE) {
        curl_easy_setopt(mHttp_handle, CURLOPT_VERBOSE, TRUE);
    } else {
        curl_easy_setopt(mHttp_handle, CURLOPT_VERBOSE, FALSE);
    }

    curl_easy_setopt(mHttp_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(mHttp_handle, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(mHttp_handle, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(mHttp_handle, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(mHttp_handle, CURLOPT_WRITEFUNCTION, write_callback);
    //  curl_easy_setopt(pHandle->http_handle, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(mHttp_handle, CURLOPT_DEBUGFUNCTION, my_trace);
    curl_easy_setopt(mHttp_handle, CURLOPT_DEBUGDATA, this);
    curl_easy_setopt(mHttp_handle, CURLOPT_HEADERFUNCTION, write_response);
    curl_easy_setopt(mHttp_handle, CURLOPT_HEADERDATA, this);
    curl_easy_setopt(mHttp_handle, CURLOPT_BUFFERSIZE, READ_BUFFER_SIZE);
    //    curl_easy_setopt(mHttp_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    curl_easy_setopt(mHttp_handle, CURLOPT_XFERINFODATA, this);
    curl_easy_setopt(mHttp_handle, CURLOPT_XFERINFOFUNCTION, xferinfo);
    curl_easy_setopt(mHttp_handle, CURLOPT_NOPROGRESS, 0);
    return 0;
}

void CURLConnection2::setInterrupt(std::atomic_bool *inter)
{
    pInterrupted = inter;
}

void CURLConnection2::SetResume(int64_t pos)
{
    mFilePos = pos;

    if (sendRange && this->mFilePos == 0) {
        curl_easy_setopt(mHttp_handle, CURLOPT_RANGE, "0-");
    } else {
        curl_easy_setopt(mHttp_handle, CURLOPT_RANGE, NULL);
        sendRange = 0;
    }

    curl_easy_setopt(mHttp_handle, CURLOPT_RESUME_FROM_LARGE, (curl_off_t) mFilePos);
}

int CURLConnection2::FillBuffer(uint32_t want, CurlMulti &multi)
{
    int64_t starTime = af_getsteady_ms();
    bool reConnect = false;

    while (RingBuffergetMaxReadSize(pRbuf) < want && RingBuffergetMaxWriteSize(pRbuf) > 0) {
        if (*(pInterrupted)) {
            AF_LOGW("FRAMEWORK_ERR_EXIT");
            return FRAMEWORK_ERR_EXIT;
        }
        //        if (mPaused) {
        //            /*
        //             * curl_easy_pause(mHttp_handle, CURLPAUSE_CONT) will sync to flush the data to write_callback, so we set mPaused to false first.
        //             */
        //            mPaused = false;
        //            curl_easy_pause(mHttp_handle, CURLPAUSE_CONT);
        //            CURL_LOGD("resume form paused\n");
        //            assert(!mPaused);
        //        }

        CURLMcode result;
        bool eof = false;

        CURLcode CURLResult = multi.performHandle(mHttp_handle, eof, result);
        if (eof) {
            return 0;
        }

        if (CURLResult != CURLE_OK) {
            if (CURLResult == CURLE_HTTP_RETURNED_ERROR) {
                long httpCode;
                curl_easy_getinfo(mHttp_handle, CURLINFO_RESPONSE_CODE, &httpCode);
                AF_LOGE("FillBuffer - Failed: HTTP returned error %ld", httpCode);
            } else {
                AF_LOGE("FillBuffer - Failed: %s(%d)", curl_easy_strerror(CURLResult), CURLResult);
            }
        }
        bool return_error = CURLResult != CURLE_OK;
        switch (CURLResult) {
            case CURLE_OK:
                break;
                // We need to check the result here as we don't want to retry on every error
            case CURLE_OPERATION_TIMEDOUT:
            case CURLE_PARTIAL_FILE:
            case CURLE_COULDNT_CONNECT:
            case CURLE_RECV_ERROR:
            case CURLE_COULDNT_RESOLVE_HOST:
                return_error = false;
                break;
            case CURLE_HTTP_RANGE_ERROR:
            case CURLE_HTTP_RETURNED_ERROR:
                // If server returns a range or http error, retry with range disabled
                if (m_bFirstLoop && mFilePos == 0 && sendRange) {
                    sendRange = 0;
                    return_error = false;
                }
                break;
            default:
                break;
        }
        if (return_error) {
            /* if we still have stuff in buffer, we are fine */
            if (RingBuffergetMaxReadSize(pRbuf)) {
                return 0;
            }
            switch (CURLResult) {
                case CURLE_RANGE_ERROR:
                    return gen_framework_errno(error_class_network, network_errno_http_range);

                case CURLE_UNSUPPORTED_PROTOCOL:
                    return gen_framework_errno(error_class_network, network_errno_unsupported);

                case CURLE_OUT_OF_MEMORY:
                    return FRAMEWORK_ERR(ENOMEM);

                case CURLE_URL_MALFORMAT:
                    return gen_framework_errno(error_class_network, network_errno_url_malformat);

                default:
                    return FRAMEWORK_ERR(EIO);
            }
        }

        if (CURLResult != CURLE_OK) {
            //            if (result != CURLM_OK) {
            //                AF_LOGE("FRAMEWORK_NET_ERR_UNKNOWN");
            //                return FRAMEWORK_NET_ERR_UNKNOWN;
            //            }

            // Close handle
            multi.removeHandle(mHttp_handle);
            reset();

            // If we got here something is wrong
            if (mPConfig && mPConfig->listener) {
                IDataSource::Listener::NetWorkRetryStatus status;

                do {
                    status = mPConfig->listener->onNetWorkRetry(getErrorCode(CURLResult));

                    if (*(pInterrupted)) {
                        return FRAMEWORK_ERR_EXIT;
                    }

                    af_msleep(10);
                } while (status == IDataSource::Listener::NetWorkRetryStatusPending);

                reConnect = true;
            } else if (mPConfig && af_getsteady_ms() - starTime > mPConfig->connect_time_out_ms) {
                AF_LOGE("FillBuffer - Reconnect failed!");
                // Reset the rest of the variables like we would in Disconnect()
                mFilePos = 0;
                // TODO: file size
                // m_fileSize = 0;
                // m_bufferSize = 0;
                //TODO: return the right errno
                return getErrorCode(CURLResult);
            }

            //    (curlContext.reTryCount)++;
            // AF_LOGE("FillBuffer - Reconnect, (re)try %i", curlContext.reTryCount);
            //TODO need change this solution: report to user.
            //sleep 10ms ， then retry connect..
            af_msleep(10);
            //            std::unique_lock<std::mutex> uMutex(curlContext.mSleepMutex);
            //            (curlContext.mSleepCondition).wait_for(uMutex, std::chrono::milliseconds(10),
            //                                                   [pConnection]() { return pInterrupted->load(); });
            // Connect + seek to current position (again)
            SetResume(mFilePos);
            multi.addHandle(mHttp_handle);
            // Return to the beginning of the loop:
            continue;
        }

        if (reConnect || m_bFirstLoop) {
            double length;

            if (curl_easy_getinfo(mHttp_handle, CURLINFO_SIZE_DOWNLOAD, &length) == CURLE_OK) {
                if (length > 0) {
                    reConnect = false;

                    if (mPConfig && mPConfig->listener) {
                        mPConfig->listener->onNetWorkConnected();
                    }
                }
            }
        }

        // We've finished out first loop
        if (m_bFirstLoop && RingBuffergetMaxReadSize(pRbuf) > 0) {
            m_bFirstLoop = 0;
        }

        switch (result) {
            case CURLM_OK: {
                // TODO: poll ringbufer
                int rc = multi.poll(10);
                if (rc == SOCKET_ERROR) {
#ifndef WIN32
                    char const *str = strerror(errno);
                    AF_LOGE("Failed with socket error:%s", str);
                    return FRAMEWORK_ERR(errno);
#endif
                }
            } break;

            case CURLM_CALL_MULTI_PERFORM: {
                // we don't keep calling here as that can easily overwrite our buffer which we want to avoid
                // docs says we should call it soon after, but aslong as we are reading data somewhere
                // this aught to be soon enough. should stay in socket otherwise
                continue;
            }

            default: {
                AF_LOGE("FillBuffer - Multi perform failed with code %d, aborting", result);
                return FRAMEWORK_NET_ERR_UNKNOWN;
            }
        }
    }

    if (mFileSize < 0) {
        double length;

        if (CURLE_OK == curl_easy_getinfo(mHttp_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length)) {
            if (length > 0.0) {
                mFileSize = mFilePos + (int64_t) length;
                //           AF_LOGE(TAG,"file size is %lld\n",curlContext.fileSize);
            } else {
                mFileSize = 0;
            }

            //        if (curlContext.fileSize == 0)
            //            curlContext.hd->is_streamed = 1;
        }
    }

    return 0;
}

int CURLConnection2::short_seek(int64_t off)
{
    uint32_t m_bufferSize = 1024 * 64;
    int ret;
    int64_t delta = off - mFilePos;

    if (delta < 0) {
        if (RingBufferSkipBytes(pRbuf, (int) delta)) {
            mFilePos = off;
            return 0;
        } else {
            return -1;
        }
    }

    if (RingBufferSkipBytes(pRbuf, (int) delta)) {
        mFilePos = off;
        return 0;
    }

    if (off < mFilePos + m_bufferSize) {
        int len = RingBuffergetMaxReadSize(pRbuf);

        if (len > 0) {
            mFilePos += len;
            RingBufferSkipBytes(pRbuf, len);
        }

        if ((ret = FillBuffer(m_bufferSize, *mMulti)) < 0) {
            if (len && !RingBufferSkipBytes(pRbuf, -len)) {
                AF_LOGE("%s - Failed to restore position after failed fill", __FUNCTION__);
            } else {
                mFilePos -= len;
            }

            return ret;
        }

        AF_LOGI("read buffer size %" PRIu32 " need is %d\n", RingBuffergetMaxReadSize(pRbuf), (int) (delta - len));

        if (!RingBufferSkipBytes(pRbuf, (int) (delta - len))) {
            AF_LOGI("%s - Failed to skip to position after having filled buffer", __FUNCTION__);

            if (len && !RingBufferSkipBytes(pRbuf, -len)) {
                AF_LOGE("%s - Failed to restore position after failed seek", __FUNCTION__);
            } else {
                mFilePos -= len;
            }

            return -1;
        }

        mFilePos = off;
        return 0;
    }

    return -1;
}

int CURLConnection2::readBuffer(void *buf, size_t size)
{
    //   double downloadSpeed;
    //   CURLcode re = curl_easy_getinfo(pEasyHandle->http_handle, CURLINFO_SPEED_DOWNLOAD, &downloadSpeed);
    /*if (re == CURLE_OK)
        av_log(mCurlhttpContext.hd,AV_LOG_DEBUG,"download speed is %f\n",downloadSpeed);*/
    uint32_t want = std::min(RingBuffergetMaxReadSize(pRbuf), (uint32_t) size);

    if (want > 0 && RingBufferReadData(pRbuf, (char *) buf, want) == want) {
        mFilePos += want;
        return want;
    }

    /* check if we finished prematurely */
    if (!still_running && (mFileSize > 0 && mFilePos != mFileSize)) {
        AF_LOGE("%s - Transfer ended before entire file was retrieved pos %lld, size %lld", __FUNCTION__, mFilePos, mFileSize);
        //   return -1;
    }

    return 0;
}

void CURLConnection2::updateSource(const string &location)
{
    curl_easy_setopt(mHttp_handle, CURLOPT_URL, location.c_str());
    mFileSize = -1;

    uri = location;
    if (reSolveList) {
        curl_slist_free_all(reSolveList);
    }

    CURLSH *sh = nullptr;
    reSolveList = CURLShareInstance::Instance()->getHosts(uri, &sh);
    assert(sh != nullptr);
    curl_easy_setopt(mHttp_handle, CURLOPT_SHARE, sh);

    if (reSolveList != nullptr) {
        curl_easy_setopt(mHttp_handle, CURLOPT_RESOLVE, reSolveList);
    }
}

void CURLConnection2::updateHeaderList(struct curl_slist *headerList)
{
    if (headerList) {
        curl_easy_setopt(mHttp_handle, CURLOPT_HTTPHEADER, headerList);
    } else {
        curl_easy_setopt(mHttp_handle, CURLOPT_HTTPHEADER, NULL);
    }
}
void CURLConnection2::disableCallBack()
{
    if (mHttp_handle) {
        curl_easy_setopt(mHttp_handle, CURLOPT_VERBOSE, FALSE);
        curl_easy_setopt(mHttp_handle, CURLOPT_WRITEDATA, nullptr);
        //   curl_easy_setopt(mHttp_handle, CURLOPT_WRITEFUNCTION, nullptr);
        //    curl_easy_setopt(mHttp_handle, CURLOPT_HEADERFUNCTION, nullptr);
        curl_easy_setopt(mHttp_handle, CURLOPT_HEADERDATA, nullptr);
        //  curl_easy_setopt(mHttp_handle, CURLOPT_SOCKOPTFUNCTION, nullptr);
        curl_easy_setopt(mHttp_handle, CURLOPT_SOCKOPTDATA, nullptr);
        // curl_easy_setopt(mHttp_handle, CURLOPT_DEBUGFUNCTION, nullptr);
        curl_easy_setopt(mHttp_handle, CURLOPT_DEBUGDATA, nullptr);
    }
}
void CURLConnection2::pause(bool pause)
{
    //   mNeedPause = pause;
}
