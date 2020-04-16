//
// Created by moqi on 2020/2/12.
//
#define LOG_TAG "CURLConnection"

#include <utils/frame_work_log.h>
#include <cerrno>
#include <utils/errors/framework_error.h>
#include <utils/timer.h>
#include "CURLConnection.h"
#include "CURLShareInstance.h"
#include <cassert>
#include <cstring>

using namespace std;
using namespace Cicada;
#define MAX_RESPONSE_SIZE 1024

#define RINGBUFFER_SIZE 1024*256
#define RINGBUFFER_BACK_SIZE 1024*512

#define MIN_SO_RCVBUF_SIZE 1024*64

#define SOCKET_ERROR (-1)

#define FITS_INT(a) (((a) <= INT_MAX) && ((a) >= INT_MIN))

#define FALSE 0L
#define TRUE  1L

static int getErrorCode(const CURLcode &CURLResult)
{
    if (CURLResult == CURLE_COULDNT_RESOLVE_HOST) {
        return gen_framework_errno(error_class_network, network_errno_resolve);
    }

    if (CURLResult == CURLE_COULDNT_CONNECT)
        return gen_framework_errno(error_class_network,
                                   network_errno_could_not_connect);

    if (CURLResult == CURLE_OPERATION_TIMEDOUT) {
        return gen_framework_errno(error_class_network,
                                   network_errno_connect_timeout);
    }

    return FRAMEWORK_ERR(EIO);
}

Cicada::CURLConnection::CURLConnection(Cicada::IDataSource::SourceConfig *pConfig)
{
    mHttp_handle = curl_easy_init();
    pRbuf = RingBufferCreate(RINGBUFFER_SIZE + RINGBUFFER_BACK_SIZE);
    RingBufferSetBackSize(pRbuf, RINGBUFFER_BACK_SIZE);
    m_bFirstLoop = 1;
    mPConfig = pConfig;

    if (pConfig) {
        so_rcv_size = pConfig->so_rcv_size;
        string &http_proxy = pConfig->http_proxy;

        if (!http_proxy.empty()) {
            AF_LOGD("http_proxy is %s\n", http_proxy.c_str());

            if (http_proxy == "never") {
                curl_easy_setopt(mHttp_handle, CURLOPT_PROXY, NULL);
            } else {
                curl_easy_setopt(mHttp_handle, CURLOPT_PROXY, http_proxy.c_str());
            }
        }

        string &refer = pConfig->refer;

        if (!refer.empty()) {
            AF_LOGD("refer is %s\n", refer.c_str());
            curl_easy_setopt(mHttp_handle, CURLOPT_REFERER, refer.c_str());
        }

        string &userAgent = pConfig->userAgent;

        if (!userAgent.empty()) {
            AF_LOGD("userAgent is %s\n", userAgent.c_str());
            curl_easy_setopt(mHttp_handle, CURLOPT_USERAGENT, userAgent.c_str());
        }

        if (pConfig->low_speed_limit && pConfig->low_speed_time_ms) {
            AF_LOGD("set low_speed_limit to %d\n", pConfig->low_speed_limit);
            AF_LOGD("set low_speed_time to %d(ms)\n", pConfig->low_speed_time_ms);
            curl_easy_setopt(mHttp_handle, CURLOPT_LOW_SPEED_LIMIT, (long) pConfig->low_speed_limit);
            curl_easy_setopt(mHttp_handle, CURLOPT_LOW_SPEED_TIME, (long) pConfig->low_speed_time_ms / 1000);
        }

        if (pConfig->connect_time_out_ms > 0) {
            AF_LOGD("set connect_time to %d(ms)\n", pConfig->connect_time_out_ms);
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
    multi_handle = curl_multi_init();
}

Cicada::CURLConnection::~CURLConnection()
{
    if (multi_handle && mHttp_handle) {
        curl_multi_remove_handle(multi_handle, mHttp_handle);
    }

    if (mHttp_handle) {
        curl_easy_cleanup(mHttp_handle);
    }

    if (pRbuf) {
        RingBufferDestroy(pRbuf);
    }

    if (pOverflowBuffer) {
        free(pOverflowBuffer);
    }

    if (response) {
        free(response);
    }

    if (multi_handle) {
        curl_multi_cleanup(multi_handle);
    }

    if (reSolveList) {
        curl_slist_free_all(reSolveList);
    }
}

void Cicada::CURLConnection::disconnect()
{
    if (multi_handle && mHttp_handle) {
        curl_multi_remove_handle(multi_handle, mHttp_handle);
    }

    RingBufferClear(pRbuf);
    responseSize = 0;
    overflowSize = 0;

    if (response) {
        response[0] = 0;
    }
}

size_t Cicada::CURLConnection::write_callback(char *buffer, size_t size, size_t nitems, void *userp)
{
    CURLConnection *pHandle = (CURLConnection *) userp;
    uint32_t amount = (uint32_t) (size * nitems);

    if (pHandle->overflowSize) {
        // we have our overflow buffer - first get rid of as much as we can
        uint32_t maxWriteable = std::min(RingBuffergetMaxWriteSize(pHandle->pRbuf), pHandle->overflowSize);

        if (maxWriteable) {
            if (RingBufferWriteData(pHandle->pRbuf, pHandle->pOverflowBuffer, maxWriteable) != maxWriteable) {
                AF_LOGE("write ring buffer error\n");
            }

            if (pHandle->overflowSize > maxWriteable) { // still have some more - copy it down
                memmove(pHandle->pOverflowBuffer, pHandle->pOverflowBuffer + maxWriteable, pHandle->overflowSize - maxWriteable);
            }

            pHandle->overflowSize -= maxWriteable;
        }
    }

    uint32_t maxWriteable = std::min(RingBuffergetMaxWriteSize(pHandle->pRbuf), amount);

    if (maxWriteable) {
        if (RingBufferWriteData(pHandle->pRbuf, buffer, maxWriteable) != maxWriteable) {
            AF_LOGE("write ring buffer error\n");
        } else {
            amount -= maxWriteable;
            buffer += maxWriteable;
        }
    }

    if (amount) {
        char *p = (char *) realloc(pHandle->pOverflowBuffer, pHandle->overflowSize + amount);

        if (p == nullptr) {
            AF_LOGE("av_realloc error\n");
            return -ENOMEM;
        }

        pHandle->pOverflowBuffer = p;
        memcpy(pHandle->pOverflowBuffer + pHandle->overflowSize, buffer, amount);
        pHandle->overflowSize += amount;
    }

    return size * nitems;
}

void Cicada::CURLConnection::debugHeader(bool in, char *data, size_t size)
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
        AF_LOGD("<= %s header \n%s", in ? "Recv" : "Send", header->c_str());
        *header = "";
    }
}

size_t Cicada::CURLConnection::write_response(void *ptr, size_t size, size_t nmemb, void *data)
{
    auto *pHandle = (CURLConnection *) data;

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

int Cicada::CURLConnection::sockopt_callback(void *clientp, curl_socket_t curlfd, curlsocktype purpose)
{
    auto *connect = (Cicada::CURLConnection *) clientp;

    if (connect->so_rcv_size <= 0) {
        return 0;
    }

    if (purpose == CURLSOCKTYPE_IPCXN) {
        int rcv_buffer = connect->so_rcv_size;

        if (rcv_buffer > 0) {
            setsockopt(curlfd, SOL_SOCKET, SO_RCVBUF, (const char *) &rcv_buffer, sizeof(rcv_buffer));
        }
    }

    /* This return code was added in libcurl 7.21.5 */
    return 0;//CURL_SOCKOPT_ALREADY_CONNECTED;
}

void Cicada::CURLConnection::setSource(const string &location, struct curl_slist *headerList)
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

int CURLConnection::my_trace(CURL *handle, curl_infotype type,
                             char *data, size_t size,
                             void *userp)
{
    const char *text;
//  int num = hnd2num(handle);
    CURLConnection *pHandle = (CURLConnection *) userp;
    (void) pHandle; /* prevent compiler warning */

    switch (type) {
        case CURLINFO_TEXT:

            AF_LOGD("== Info: %s", data);
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

    //  AF_LOGD("%s\n", text);
    return 0;
}

int Cicada::CURLConnection::esayHandle_set_common_opt()
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
//For https ignore CA certificates
    curl_easy_setopt(mHttp_handle, CURLOPT_SSL_VERIFYPEER, FALSE);
    //   curl_easy_setopt(mHttp_handle, CURLOPT_SSL_VERIFYHOST, FALSE);
    return 0;
}

void CURLConnection::setInterrupt(std::atomic_bool *inter)
{
    pInterrupted = inter;
}

void CURLConnection::SetResume(int64_t pos)
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

void CURLConnection::start()
{
    curl_multi_add_handle(multi_handle, mHttp_handle);
}

int CURLConnection::FillBuffer(uint32_t want)
{
    fd_set fdread;
    fd_set fdwrite;
    fd_set fdexcep;
    int64_t starTime = af_getsteady_ms();
    bool reConnect = false;

    while (RingBuffergetMaxReadSize(pRbuf) < want &&
            RingBuffergetMaxWriteSize(pRbuf) > 0) {
        if (*(pInterrupted)) {
            AF_LOGW("FRAMEWORK_ERR_EXIT");
            return FRAMEWORK_ERR_EXIT;
        }

        /* if there is data in overflow buffer, try to use that first */
        if (overflowSize) {
            uint32_t amount = std::min(RingBuffergetMaxWriteSize(pRbuf),
                                       overflowSize);
            RingBufferWriteData(pRbuf, pOverflowBuffer, amount);

            if (amount < overflowSize)
                memcpy(pOverflowBuffer, pOverflowBuffer + amount,
                       overflowSize - amount);

            overflowSize -= amount;
            char *p = static_cast<char *>(realloc(pOverflowBuffer,
                                                  overflowSize));

            if (p == nullptr) {
                //    AF_LOGE("av_realloc error\n");
                //     return -ENOMEM;
            }

            pOverflowBuffer = p;
            continue;
        }

        CURLMcode result = curl_multi_perform(multi_handle, &still_running);

        if (!still_running) {
            if (result != CURLM_OK) {
                AF_LOGE("FRAMEWORK_NET_ERR_UNKNOWN");
                return FRAMEWORK_NET_ERR_UNKNOWN;
            }

            /* if we still have stuff in buffer, we are fine */
            if (RingBuffergetMaxReadSize(pRbuf)) {
                return 0;
            }

            /* verify that we are actually okey */
            int msgs;
            CURLcode CURLResult = CURLE_OK;
            CURLMsg *msg;

            while ((msg = curl_multi_info_read(multi_handle, &msgs))) {
                if (msg->msg == CURLMSG_DONE) {
                    if (msg->data.result == CURLE_OK) {
                        return 0;
                    }

                    if (msg->data.result == CURLE_HTTP_RETURNED_ERROR) {
                        long httpCode;
                        curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &httpCode);
                        AF_LOGE("FillBuffer - Failed: HTTP returned error %ld", httpCode);
                    } else {
                        AF_LOGE("FillBuffer - Failed: %s(%d)", curl_easy_strerror(msg->data.result), msg->data.result);
                    }

                    // We need to check the result here as we don't want to retry on every error
                    if ((msg->data.result == CURLE_OPERATION_TIMEDOUT ||
                            msg->data.result == CURLE_PARTIAL_FILE ||
                            msg->data.result == CURLE_COULDNT_CONNECT ||
                            msg->data.result == CURLE_RECV_ERROR ||
                            msg->data.result == CURLE_COULDNT_RESOLVE_HOST) /*&&
                              !pEasyHandle->m_bFirstLoop*/) {
                        CURLResult = msg->data.result;
                    } else if ((msg->data.result == CURLE_HTTP_RANGE_ERROR ||
                                msg->data.result == CURLE_HTTP_RETURNED_ERROR) &&
                               m_bFirstLoop &&
                               mFilePos == 0 &&
                               sendRange) {
                        // If server returns a range or http error, retry with range disabled
                        CURLResult = msg->data.result;
                        sendRange = 0;
                    } else {
                        //TODO: return the right errno
                        //   AF_LOGE("url is %s\n", curlContext.location.c_str());
                        switch (msg->data.result) {
                            case CURLE_RANGE_ERROR:
                                return gen_framework_errno(error_class_network, network_errno_http_range);

                            case CURLE_UNSUPPORTED_PROTOCOL:
                                return gen_framework_errno(error_class_network, network_errno_unsupported);

                            case CURLE_OUT_OF_MEMORY:
                                return FRAMEWORK_ERR(ENOMEM);

                            default:
                                return FRAMEWORK_ERR(EIO);
                        }
                    }
                }
            }

            // Don't retry when we didn't "see" any error
#ifndef WIN32

            if (CURLResult == CURLE_OK) {
                return FRAMEWORK_NET_ERR_UNKNOWN;
            }

#endif
            // Close handle
            disconnect();

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
            } else if (af_getsteady_ms() - starTime > mPConfig->connect_time_out_ms) {
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
            curl_multi_add_handle(multi_handle, mHttp_handle);
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
                int maxfd = -1;
                FD_ZERO(&fdread);
                FD_ZERO(&fdwrite);
                FD_ZERO(&fdexcep);
                // get file descriptors from the transfers
                curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
                long timeout = 0;

                if (CURLM_OK != curl_multi_timeout(multi_handle, &timeout) || timeout == -1 ||
                        timeout < 200) {
                    timeout = 200;
                }

                //    XbmcThreads::EndTime endTime(timeout);
                // TODO: count time
                int rc;
                int count = 10;

                do {
                    //  unsigned int time_left = endTime.MillisLeft();
                    //  struct timeval t = { (int)time_left / 1000, ((int)time_left % 1000) * 1000 };
                    struct timeval t = {0, 10000};
                    // Wait until data is available or a timeout occurs.
                    rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &t);
                    count--;
                } while (rc == SOCKET_ERROR && errno == EINTR && count > 0);

                if (rc == SOCKET_ERROR) {
#ifndef WIN32
                    char const *str = strerror(errno);
                    AF_LOGE("Failed with socket error:%s", str);
                    return FRAMEWORK_ERR(errno);
#endif
                }
            }
            break;

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

        if (CURLE_OK ==
                curl_easy_getinfo(mHttp_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length)) {
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

int CURLConnection::short_seek(int64_t off)
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

        if ((ret = FillBuffer(m_bufferSize)) < 0) {
            if (len && !RingBufferSkipBytes(pRbuf, -len)) {
                AF_LOGE("%s - Failed to restore position after failed fill", __FUNCTION__);
            } else {
                mFilePos -= len;
            }

            return ret;
        }

        AF_LOGI("read buffer size %d need is %d\n", RingBuffergetMaxReadSize(pRbuf), delta - len);

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

int CURLConnection::readBuffer(void *buf, size_t size)
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
    if (!still_running &&
            (mFileSize > 0 && mFilePos != mFileSize)) {
        AF_LOGE("%s - Transfer ended before entire file was retrieved pos %lld, size %lld",
                __FUNCTION__, mFilePos, mFileSize);
        //   return -1;
    }

    return 0;
}

void CURLConnection::updateSource(const string &location)
{
    curl_easy_setopt(mHttp_handle, CURLOPT_URL, location.c_str());
    mFileSize = -1;
}
