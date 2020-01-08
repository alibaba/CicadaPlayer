//
// Created by moqi on 2018/1/25.
//
#define LOG_TAG "CurlDataSource"

#include "curl_data_source.h"
#include <pthread.h>

#include <utils/frame_work_log.h>
#include <utils/errors/framework_error.h>
#include <cstdlib>
#include <thread>
#include <utils/timer.h>
#include "utils/CicadaJSON.h"
#include "utils/AsyncJob.h"
#include "data_source/DataSourceUtils.h"
#include "CURLShareInstance.h"

#ifdef WIN32
    #include <winsock2.h>
#endif

#include <cerrno>
#include <utils/CicadaUtils.h>
#include <openssl/opensslv.h>
#include <cassert>
#include <cstring>



// TODO: move to another file
#if defined(WIN32) || defined(WIN64)
    #define strcasecmp _stricmp
    #define strncasecmp _strnicmp
#endif /* Def WIN32 or Def WIN64 */

#define SOCKET_ERROR (-1)


#define FALSE 0L
#define TRUE  1L


#define MAX_RESPONSE_SIZE 1024

//#define false 0
//#define true  1
#define RINGBUFFER_SIZE 1024*16

#define MIN_SO_RCVBUF_SIZE 1024*64

#define MAX_RETRY_COUNT 3
#define MIN(a, b) ((a)<(b)?(a):(b))
#define FITS_INT(a) (((a) <= INT_MAX) && ((a) >= INT_MIN))

//static pthread_mutex_t g_mutex; ///< we have nowhere to destroy this.
//static int g_lock_inited = 0;

static pthread_once_t once = PTHREAD_ONCE_INIT;
using namespace Cicada;

CurlDataSource CurlDataSource::se(0);
using std::string;

static size_t write_callback(char *buffer, size_t size, size_t nitems, void *userp)
{
    if (userp == NULL) { return 0; }

    CURLConnection *pHandle = (CURLConnection *) userp;
    uint32_t amount = (uint32_t) (size * nitems);

    //   av_log(pHandle->parent,AV_LOG_ERROR,"read size %u\n",amount);
    if (pHandle->overflowSize) {
        // we have our overflow buffer - first get rid of as much as we can
        uint32_t maxWriteable = MIN(RingBuffergetMaxWriteSize(pHandle->pRbuf),
                                    pHandle->overflowSize);

        if (maxWriteable) {
            if (RingBufferWriteData(pHandle->pRbuf, pHandle->pOverflowBuffer, maxWriteable) !=
                    maxWriteable) {
                AF_LOGE("write ring buffer error\n");
            }

            if (pHandle->overflowSize > maxWriteable) { // still have some more - copy it down
                memmove(pHandle->pOverflowBuffer, pHandle->pOverflowBuffer + maxWriteable,
                        pHandle->overflowSize - maxWriteable);
            }

            pHandle->overflowSize -= maxWriteable;
        }
    }

    uint32_t maxWriteable = MIN(RingBuffergetMaxWriteSize(pHandle->pRbuf), amount);

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

        if (p == NULL) {
            AF_LOGE("av_realloc error\n");
            return -ENOMEM;
        }

        pHandle->pOverflowBuffer = p;
        memcpy(pHandle->pOverflowBuffer + pHandle->overflowSize, buffer, amount);
        pHandle->overflowSize += amount;
    }

    return size * nitems;
}

static
int my_trace(CURL *handle, curl_infotype type,
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
            pHandle->outHeader += string(data, size);

            if (pHandle->outHeader.length() < 4) {
                return 0;
            }

            if (pHandle->outHeader.compare(pHandle->outHeader.length() - 4, 4, "\r\n\r\n") == 0) {
                AF_LOGD("=> Send header \n%s", pHandle->outHeader.c_str());
                pHandle->outHeader = "";
            }

            return 0;

        case CURLINFO_DATA_OUT:
            text = "=> Send data";
            break;

        case CURLINFO_SSL_DATA_OUT:
            text = "=> Send SSL data";
            break;

        case CURLINFO_HEADER_IN:
            pHandle->inHeader += string(data, size);

            if (pHandle->inHeader.length() < 4) {
                return 0;
            }

            if (pHandle->inHeader.compare(pHandle->inHeader.length() - 4, 4, "\r\n\r\n") == 0) {
                AF_LOGD("<= Recv header \n%s", pHandle->inHeader.c_str());
                pHandle->inHeader = "";
            }

            return 0;

        case CURLINFO_DATA_IN:
            text = "<= Recv data";
            break;

        case CURLINFO_SSL_DATA_IN:
            text = "<= Recv SSL data";
            break;
    }

    AF_LOGD("%s\n", text);
    return 0;
}

static size_t write_response(void *ptr, size_t size, size_t nmemb, void *data)
{
    CURLConnection *pHandle = (CURLConnection *) data;

    if (pHandle->response == NULL) {
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

static int sockopt_callback(void *clientp, curl_socket_t curlfd,
                            curlsocktype purpose)
{
    CURLConnection *connect = (CURLConnection *) clientp;

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

static int esayHandle_set_common_opt(CURLConnection *pHandle)
{
    if (log_get_level() >= AF_LOG_LEVEL_TRACE) {
        curl_easy_setopt(pHandle->http_handle, CURLOPT_VERBOSE, TRUE);
    } else {
        curl_easy_setopt(pHandle->http_handle, CURLOPT_VERBOSE, FALSE);
    }

    curl_easy_setopt(pHandle->http_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(pHandle->http_handle, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(pHandle->http_handle, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(pHandle->http_handle, CURLOPT_WRITEDATA, pHandle);
    curl_easy_setopt(pHandle->http_handle, CURLOPT_WRITEFUNCTION, write_callback);
    //  curl_easy_setopt(pHandle->http_handle, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(pHandle->http_handle, CURLOPT_DEBUGFUNCTION, my_trace);
    curl_easy_setopt(pHandle->http_handle, CURLOPT_DEBUGDATA, pHandle);
    curl_easy_setopt(pHandle->http_handle, CURLOPT_HEADERFUNCTION, write_response);
    curl_easy_setopt(pHandle->http_handle, CURLOPT_HEADERDATA, pHandle);
//For https ignore CA certificates
    curl_easy_setopt(pHandle->http_handle, CURLOPT_SSL_VERIFYPEER, FALSE);
    curl_easy_setopt(pHandle->http_handle, CURLOPT_SSL_VERIFYHOST, FALSE);
    return 0;
}

static void setUrl(CURL *http_handle, struct curl_slist **reSolveList, const string &uri)
{
    curl_easy_setopt(http_handle, CURLOPT_URL, uri.c_str());
    CURLSH *sh = nullptr;

    if (*reSolveList) {
        curl_slist_free_all(*reSolveList);
        *reSolveList = nullptr;
    }

    *reSolveList = CURLShareInstance::Instance()->getHosts(uri, &sh);
    assert(sh != nullptr);
    curl_easy_setopt(http_handle, CURLOPT_SHARE, sh);

    if (*reSolveList != nullptr) {
        curl_easy_setopt(http_handle, CURLOPT_RESOLVE, *reSolveList);
    }
}

static CURLConnection *initConnection(CURLHTTPContext &curlContext)
{
    CURLConnection *pHandle = new CURLConnection();
    pHandle->http_handle = curl_easy_init();
    pHandle->pRbuf = RingBufferCreate(RINGBUFFER_SIZE);
    pHandle->m_bFirstLoop = 1;
    pHandle->so_rcv_size = curlContext.pConfig->so_rcv_size;
    esayHandle_set_common_opt(pHandle);
    curl_easy_setopt(pHandle->http_handle, CURLOPT_SOCKOPTFUNCTION, sockopt_callback);
    curl_easy_setopt(pHandle->http_handle, CURLOPT_SOCKOPTDATA, pHandle);
    string &http_proxy = curlContext.pConfig->http_proxy;

    if (!http_proxy.empty()) {
        AF_LOGD("http_proxy is %s\n", http_proxy.c_str());

        if (http_proxy == "never") {
            curl_easy_setopt(pHandle->http_handle, CURLOPT_PROXY, NULL);
        } else {
            curl_easy_setopt(pHandle->http_handle, CURLOPT_PROXY, http_proxy.c_str());
        }
    }

    string &refer = curlContext.pConfig->refer;

    if (!refer.empty()) {
        AF_LOGD("refer is %s\n", refer.c_str());
        curl_easy_setopt(pHandle->http_handle, CURLOPT_REFERER, refer.c_str());
    }

    string &userAgent = curlContext.pConfig->userAgent;

    if (!userAgent.empty()) {
        AF_LOGD("userAgent is %s\n", userAgent.c_str());
        curl_easy_setopt(pHandle->http_handle, CURLOPT_USERAGENT, userAgent.c_str());
    }

    if (curlContext.headerList) {
        curl_easy_setopt(pHandle->http_handle, CURLOPT_HTTPHEADER, curlContext.headerList);
    } else {
        curl_easy_setopt(pHandle->http_handle, CURLOPT_HTTPHEADER, NULL);
    }

    if (curlContext.pConfig->low_speed_limit && curlContext.pConfig->low_speed_time_ms) {
        AF_LOGD("set low_speed_limit to %d\n", curlContext.pConfig->low_speed_limit);
        AF_LOGD("set low_speed_time to %d(ms)\n", curlContext.pConfig->low_speed_time_ms);
        curl_easy_setopt(pHandle->http_handle, CURLOPT_LOW_SPEED_LIMIT, (long) curlContext.pConfig->low_speed_limit);
        curl_easy_setopt(pHandle->http_handle, CURLOPT_LOW_SPEED_TIME, (long) curlContext.pConfig->low_speed_time_ms / 1000);
    }

    if (curlContext.pConfig->connect_time_out_ms > 0) {
        AF_LOGD("set connect_time to %d(ms)\n", curlContext.pConfig->connect_time_out_ms);
        curl_easy_setopt(pHandle->http_handle, CURLOPT_CONNECTTIMEOUT, (long) curlContext.pConfig->connect_time_out_ms / 1000);
    }

    setUrl(pHandle->http_handle, &curlContext.reSolveList, curlContext.location);
    pHandle->multi_handle = curl_multi_init();
    return pHandle;
}


static void curl_destroy_easy_handle(CURL *pHandle)
{
    curl_easy_cleanup(pHandle);
    return;
}

static void destroyConnection(CURLConnection *pHandle)
{
    if (pHandle->http_handle) {
        curl_destroy_easy_handle(pHandle->http_handle);
        pHandle->http_handle = NULL;
    }

    if (pHandle->pRbuf) {
        RingBufferDestroy(pHandle->pRbuf);
        pHandle->pRbuf = nullptr;
    }

    if (pHandle->pOverflowBuffer) {
        free(pHandle->pOverflowBuffer);
        pHandle->pOverflowBuffer = nullptr;
    }

    if (pHandle->response) {
        free(pHandle->response);
        pHandle->response = nullptr;
    }

    if (pHandle->multi_handle) {
        curl_multi_cleanup(pHandle->multi_handle);
        pHandle->multi_handle = nullptr;
    }

    delete pHandle;
}

void SetResume(CURLConnection *pConnection)
{
    if (pConnection->sendRange && pConnection->filePos == 0) {
        curl_easy_setopt(pConnection->http_handle, CURLOPT_RANGE, "0-");
    } else {
        curl_easy_setopt(pConnection->http_handle, CURLOPT_RANGE, NULL);
        pConnection->sendRange = 0;
    }

    curl_easy_setopt(pConnection->http_handle, CURLOPT_RESUME_FROM_LARGE, (curl_off_t) pConnection->filePos);
}

/* use to attempt to fill the read buffer up to requested number of bytes */
int CurlDataSource::FillBuffer(CURLHTTPContext &curlContext, CURLConnection *pConnection, uint32_t want)
{
    fd_set fdread;
    fd_set fdwrite;
    fd_set fdexcep;
    int64_t starTime = af_getsteady_ms();
    bool reConnect = false;

    while (RingBuffergetMaxReadSize(pConnection->pRbuf) < want &&
            RingBuffergetMaxWriteSize(pConnection->pRbuf) > 0) {
        if (*(pConnection->pInterrupted)) {
            AF_LOGW("FRAMEWORK_ERR_EXIT");
            return FRAMEWORK_ERR_EXIT;
        }

        /* if there is data in overflow buffer, try to use that first */
        if (pConnection->overflowSize) {
            uint32_t amount = MIN(RingBuffergetMaxWriteSize(pConnection->pRbuf),
                                  pConnection->overflowSize);
            RingBufferWriteData(pConnection->pRbuf, pConnection->pOverflowBuffer, amount);

            if (amount < pConnection->overflowSize)
                memcpy(pConnection->pOverflowBuffer, pConnection->pOverflowBuffer + amount,
                       pConnection->overflowSize - amount);

            pConnection->overflowSize -= amount;
            pConnection->pOverflowBuffer = static_cast<char *>(realloc(pConnection->pOverflowBuffer,
                                           pConnection->overflowSize));
            continue;
        }

        CURLMcode result = curl_multi_perform(pConnection->multi_handle, &pConnection->still_running);

        if (!pConnection->still_running) {
            if (result != CURLM_OK) {
                AF_LOGE("FRAMEWORK_NET_ERR_UNKNOWN");
                return FRAMEWORK_NET_ERR_UNKNOWN;
            }

            /* if we still have stuff in buffer, we are fine */
            if (RingBuffergetMaxReadSize(pConnection->pRbuf)) {
                return 0;
            }

            /* verify that we are actually okey */
            int msgs;
            CURLcode CURLResult = CURLE_OK;
            CURLMsg *msg;

            while ((msg = curl_multi_info_read(pConnection->multi_handle, &msgs))) {
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
                               pConnection->m_bFirstLoop &&
                               pConnection->filePos == 0 &&
                               pConnection->sendRange) {
                        // If server returns a range or http error, retry with range disabled
                        CURLResult = msg->data.result;
                        pConnection->sendRange = 0;
                    } else {
                        //TODO: return the right errno
                        AF_LOGE("url is %s\n", curlContext.location.c_str());

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
            if (pConnection->multi_handle && pConnection->http_handle) {
                curl_multi_remove_handle(pConnection->multi_handle, pConnection->http_handle);
            }

            // Reset all the stuff like we would in Disconnect()
            RingBufferClear(pConnection->pRbuf);
            pConnection->overflowSize = 0;

            // If we got here something is wrong
            if (mConfig.listener) {
                Listener::NetWorkRetryStatus status;

                do {
                    status = mConfig.listener->onNetWorkRetry(getErrorCode(CURLResult));

                    if (*(pConnection->pInterrupted)) {
                        return FRAMEWORK_ERR_EXIT;
                    }

                    af_msleep(10);
                } while (status == Listener::NetWorkRetryStatusPending);

                reConnect = true;
            } else if (af_getsteady_ms() - starTime > curlContext.pConfig->connect_time_out_ms) {
                AF_LOGE("FillBuffer - Reconnect failed!");
                // Reset the rest of the variables like we would in Disconnect()
                pConnection->filePos = 0;
                // TODO: file size
                // m_fileSize = 0;
                // m_bufferSize = 0;
                //TODO: return the right errno
                return getErrorCode(CURLResult);
            }

            (curlContext.reTryCount)++;
            AF_LOGE("FillBuffer - Reconnect, (re)try %i", curlContext.reTryCount);
            //TODO need change this solution: report to user.
            //sleep 10ms ， then retry connect..
            std::unique_lock<std::mutex> uMutex(curlContext.mSleepMutex);
            (curlContext.mSleepCondition).wait_for(uMutex, std::chrono::milliseconds(10),
            [pConnection]() { return pConnection->pInterrupted->load(); });
            // Connect + seek to current position (again)
            SetResume(pConnection);
            curl_multi_add_handle(pConnection->multi_handle, pConnection->http_handle);
            // Return to the beginning of the loop:
            continue;
        }

        if (reConnect || pConnection->m_bFirstLoop) {
            double length;

            if (curl_easy_getinfo(pConnection->http_handle, CURLINFO_SIZE_DOWNLOAD, &length) == CURLE_OK) {
                if (length > 0) {
                    reConnect = false;

                    if (mConfig.listener) {
                        mConfig.listener->onNetWorkConnected();
                    }
                }
            }
        }

        // We've finished out first loop
        if (pConnection->m_bFirstLoop && RingBuffergetMaxReadSize(pConnection->pRbuf) > 0) {
            pConnection->m_bFirstLoop = 0;
        }

        switch (result) {
            case CURLM_OK: {
                int maxfd = -1;
                FD_ZERO(&fdread);
                FD_ZERO(&fdwrite);
                FD_ZERO(&fdexcep);
                // get file descriptors from the transfers
                curl_multi_fdset(pConnection->multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
                long timeout = 0;

                if (CURLM_OK != curl_multi_timeout(pConnection->multi_handle, &timeout) || timeout == -1 ||
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

    return 0;
}

int CurlDataSource::getErrorCode(const CURLcode &CURLResult) const
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

// return -2 to retry
int CurlDataSource::curl_connect(CURLHTTPContext &curlContext, CURLConnection *pConnection)
{
    int ret;
    char *location = NULL;
    char *ipstr = NULL;
    double length;
    long response;
    AF_LOGD("start connect\n");
    SetResume(pConnection);
    curl_multi_add_handle(pConnection->multi_handle, pConnection->http_handle);

    if ((ret = FillBuffer(curlContext, pConnection, 1)) < 0) {
        AF_LOGE("Connect, didn't get any data from stream.");
        return ret;
    }

    AF_LOGD("connected\n");

    if (CURLE_OK ==
            curl_easy_getinfo(pConnection->http_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length)) {
        if (length < 0) {
            length = 0.0;
        }

        if (length > 0.0) {
            curlContext.fileSize = pConnection->filePos + (int64_t) length;
            //           AF_LOGE(TAG,"file size is %lld\n",curlContext.fileSize);
        }

//        if (curlContext.fileSize == 0)
//            curlContext.hd->is_streamed = 1;
    }

    if (CURLE_OK ==
            curl_easy_getinfo(pConnection->http_handle, CURLINFO_EFFECTIVE_URL, &location)) {
        if (location) {
            curlContext.location = location;
        } else {
            curlContext.location = "";
        }
    }

    if (CURLE_OK == curl_easy_getinfo(pConnection->http_handle, CURLINFO_PRIMARY_IP, &ipstr)) {
        if (ipstr) {
            curlContext.ipStr = ipstr;
        } else {
            curlContext.ipStr = "";
        }
    }

    if (CURLE_OK ==
            curl_easy_getinfo(pConnection->http_handle, CURLINFO_RESPONSE_CODE, &response)) {
        AF_LOGI("CURLINFO_RESPONSE_CODE is %d", response);

        if (response >= 400) {
            return gen_framework_http_errno((int) response);
        }
    }

    return 0;
}

static void curl_disconnect(CURLConnection *pConnection)
{
    if (pConnection == NULL) {
        return;
    }

    CURLM *multi_handle = pConnection->multi_handle;

    if (multi_handle && pConnection->http_handle) {
        curl_multi_remove_handle(multi_handle, pConnection->http_handle);
    }

    RingBufferClear(pConnection->pRbuf);

    if (pConnection->pOverflowBuffer) {
        free(pConnection->pOverflowBuffer);
        pConnection->pOverflowBuffer = NULL;
        pConnection->overflowSize = 0;
    }

    pConnection->filePos = 0;
}


int CurlDataSource::curl_easy_seek(CURLHTTPContext &curlContext, int64_t off)
{
    CURLConnection *pConnection = curlContext.pConnection;
    uint32_t m_bufferSize = 1024 * 512;
    int ret;

    if (FITS_INT(off - pConnection->filePos) &&
            RingBufferSkipBytes(pConnection->pRbuf, (int) (off - pConnection->filePos))) {
        pConnection->filePos = off;
        return 0;
    }

    if (off > pConnection->filePos && off < pConnection->filePos + m_bufferSize) {
        int len = RingBuffergetMaxReadSize(pConnection->pRbuf);
        pConnection->filePos += len;
        RingBufferSkipBytes(pConnection->pRbuf, len);

        if ((ret = FillBuffer(curlContext, pConnection, m_bufferSize)) < 0) {
            if (!RingBufferSkipBytes(pConnection->pRbuf, -len)) {
                AF_LOGE("%s - Failed to restore position after failed fill", __FUNCTION__);
            } else {
                pConnection->filePos -= len;
            }

            return ret;
        }

        if (!FITS_INT(off - pConnection->filePos) ||
                !RingBufferSkipBytes(pConnection->pRbuf, (int) (off - pConnection->filePos))) {
            AF_LOGE("%s - Failed to skip to position after having filled buffer", __FUNCTION__);

            if (!RingBufferSkipBytes(pConnection->pRbuf, -len)) {
                AF_LOGE("%s - Failed to restore position after failed seek", __FUNCTION__);
            } else {
                pConnection->filePos -= len;
            }

            return -1;
        }

        pConnection->filePos = off;
        return 0;
    }

    return -1;
}

static void init_curl()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
#if (OPENSSL_VERSION_NUMBER < 0x10100000L)
    openssl_thread_setup();
#endif
}

namespace Cicada {

    CURLHTTPContext::~CURLHTTPContext()
    {
        if (pConnection) {
            destroyConnection(pConnection);
        }

        if (headerList) {
            curl_slist_free_all(headerList);
        }

        if (reSolveList) {
            curl_slist_free_all(reSolveList);
        }
    }

    CurlDataSource::CurlDataSource(const string &url) : IDataSource(url)
    {
        pthread_once(&once, init_curl);
        mCurlhttpContext.fileSize = -1;
    }

    CurlDataSource::~CurlDataSource()
    {
        AF_LOGI("!~CurlDataSource");
        Interrupt(true);
        Close();
    }

    int CurlDataSource::Open(int flags)
    {
        // TODO: deal with ret
        mOpenTimeMS = af_gettime_relative() / 1000;
        bool isRTMP = mUri.compare(0, 7, "rtmp://") == 0;
        mCurlhttpContext.location = (isRTMP ? (mUri + " live=1").c_str() : mUri.c_str());
        mCurlhttpContext.pConfig = &mConfig;

        if (mCurlhttpContext.headerList) {
            curl_slist_free_all(mCurlhttpContext.headerList);
            mCurlhttpContext.headerList = NULL;
        }

        std::vector<std::string> &customHeaders = mConfig.customHeaders;

        for (string &item : customHeaders) {
            if (!item.empty()) {
                mCurlhttpContext.headerList = curl_slist_append(mCurlhttpContext.headerList, item.c_str());
            }
        }

        if (mCurlhttpContext.pConfig->so_rcv_size >= MIN_SO_RCVBUF_SIZE) {
            mCurlhttpContext.pConfig->so_rcv_size = mCurlhttpContext.pConfig->so_rcv_size >> 12;
            mCurlhttpContext.pConfig->so_rcv_size = mCurlhttpContext.pConfig->so_rcv_size << 12;
            AF_LOGI("so_rcv_size is %d\n", mCurlhttpContext.pConfig->so_rcv_size);
        } else if (mCurlhttpContext.pConfig->so_rcv_size > 0) {
            AF_LOGI("so_rcv_size too small\n");
            mCurlhttpContext.pConfig->so_rcv_size = 0;
        }

        {
            std::lock_guard<std::mutex> lock(mMutex);
            mCurlhttpContext.pConnection = initConnection(mCurlhttpContext);
            mCurlhttpContext.pConnection->pInterrupted = &mInterrupt;
        }

        int ret = curl_connect(mCurlhttpContext, mCurlhttpContext.pConnection);
        mOpenTimeMS = af_gettime_relative() / 1000 - mOpenTimeMS;

        if (ret >= 0) {
            fillConnectInfo();
        }

        return ret;
    }

    int CurlDataSource::Open(const string &url)
    {
        if (mCurlhttpContext.pConnection == nullptr) {
            mUri = url;
            return Open(0);
        }

//        if (curlContext.still_running){
//            return -1;
//        }
        mOpenTimeMS = af_gettime_relative() / 1000;

        if (mCurlhttpContext.pConnection->multi_handle && mCurlhttpContext.pConnection->http_handle)
            curl_multi_remove_handle(mCurlhttpContext.pConnection->multi_handle,
                                     mCurlhttpContext.pConnection->http_handle);

        RingBufferClear(mCurlhttpContext.pConnection->pRbuf);
        mCurlhttpContext.pConnection->filePos = 0;
        mCurlhttpContext.pConnection->responseSize = 0;
        mCurlhttpContext.pConnection->overflowSize = 0;

        if (mCurlhttpContext.pConnection->response) {
            mCurlhttpContext.pConnection->response[0] = 0;
        }

        bool isRTMP = url.compare(0, 7, "rtmp://") == 0;
        mCurlhttpContext.location = (isRTMP ? (url + " live=1").c_str() : url.c_str());
        // only change url, don,t change share and resolve
        curl_easy_setopt(mCurlhttpContext.pConnection->http_handle, CURLOPT_URL, mCurlhttpContext.location.c_str());
        int ret = curl_connect(mCurlhttpContext, mCurlhttpContext.pConnection);
        mOpenTimeMS = af_gettime_relative() / 1000 - mOpenTimeMS;

        if (ret >= 0) {
            fillConnectInfo();
        }

        return ret;
    }

    void CurlDataSource::Close()
    {
        std::lock_guard<std::mutex> lock(mMutex);
        CURLConnection *deleteConnection = mCurlhttpContext.pConnection;
        mCurlhttpContext.pConnection = nullptr;

        if (deleteConnection) {
            AsyncJob::Instance()->addJob([deleteConnection] {
                curl_disconnect(deleteConnection);
                destroyConnection(deleteConnection);
            });
        }
    }

    int64_t CurlDataSource::Seek(int64_t offset, int whence)
    {
        //AF_LOGD("CurlDataSource::Seek position is %lld,when is %d",offset,whence);
        CURLConnection *pConnection = mCurlhttpContext.pConnection;

        if (whence == SEEK_SIZE) {
            return mCurlhttpContext.fileSize;
        } else if ((whence == SEEK_CUR && offset == 0) ||
                   (whence == SEEK_SET && offset == pConnection->filePos)) {
            return pConnection->filePos;
        } else if ((mCurlhttpContext.fileSize <= 0 && whence == SEEK_END) /*|| h->is_streamed*/) {
            return FRAMEWORK_ERR(ENOSYS);
        }

        if (whence == SEEK_CUR) {
            offset += pConnection->filePos;
        } else if (whence == SEEK_END) {
            offset += mCurlhttpContext.fileSize;
        } else if (whence != SEEK_SET) {
            return FRAMEWORK_ERR(EINVAL);
        }

        if (offset < 0) {
            return -(EINVAL);
        }

        if (offset == pConnection->filePos) {
            return offset;
        }

        //first seek in cache
        if (curl_easy_seek(mCurlhttpContext, offset) >= 0) {
            return offset;
        }

        int64_t ret = TrySeekByNewConnection(offset);
        return (ret >= 0) ? ret : -1;
    }

    int64_t CurlDataSource::TrySeekByNewConnection(int64_t offset)
    {
        // try seek use a new connection
        CURLConnection *pConnection_s = initConnection(mCurlhttpContext);
        pConnection_s->pInterrupted = &mInterrupt;
        pConnection_s->filePos = offset;
        int ret = curl_connect(mCurlhttpContext, pConnection_s);

        if (ret >= 0) {
            std::lock_guard<std::mutex> lock(mMutex);
            // try seek ok, use the new connection
            curl_disconnect(mCurlhttpContext.pConnection);
            destroyConnection(mCurlhttpContext.pConnection);
            mCurlhttpContext.pConnection = pConnection_s;
            return offset;
        }

        // try seek faild, use the old connection
        curl_disconnect(pConnection_s);
        destroyConnection(pConnection_s);
        return ret;
    }

    int CurlDataSource::Read(void *buf, size_t size)
    {
        CURLConnection *pConnection = mCurlhttpContext.pConnection;
        int ret = 0;

        /* only request 1 byte, for truncated reads (only if not eof) */
        if ((mCurlhttpContext.fileSize <= 0 || pConnection->filePos < mCurlhttpContext.fileSize) &&
                (ret = FillBuffer(mCurlhttpContext, pConnection, 1)) < 0) {
            if (ret < 0) {
                return ret;
            }

            return 0;
        }

        //   double downloadSpeed;
        //   CURLcode re = curl_easy_getinfo(pEasyHandle->http_handle, CURLINFO_SPEED_DOWNLOAD, &downloadSpeed);
        /*if (re == CURLE_OK)
            av_log(mCurlhttpContext.hd,AV_LOG_DEBUG,"download speed is %f\n",downloadSpeed);*/
        uint32_t want = MIN(RingBuffergetMaxReadSize(pConnection->pRbuf), size);

        if (RingBufferReadData(pConnection->pRbuf, (char *) buf, want) == want) {
            pConnection->filePos += want;
            return want;
        }

        /* check if we finished prematurely */
        if (!pConnection->still_running &&
                (mCurlhttpContext.fileSize <= 0 || pConnection->filePos != mCurlhttpContext.fileSize)) {
            AF_LOGE("%s - Transfer ended before entire file was retrieved pos %lld, size %lld",
                    __FUNCTION__,
                    pConnection->filePos, mCurlhttpContext.fileSize);
            return -1;
        }

        return 0;
    }

    string CurlDataSource::GetOption(const string &key)
    {
        std::lock_guard<std::mutex> lock(mMutex);

        if (key == "responseInfo") {
            if (mCurlhttpContext.pConnection && mCurlhttpContext.pConnection->response) {
                CicadaJSONItem Json;
                Json.addValue("response", mCurlhttpContext.pConnection->response);
                return Json.printJSON();
            } else {
                return "";
            }
        }

        if (key == "connectInfo") {
            return mConnectInfo;
        }

        return IDataSource::GetOption(key);
    }

    void CurlDataSource::fillConnectInfo()
    {
        CURLConnection *pConnection = mCurlhttpContext.pConnection;
        CicadaJSONItem Json;
        Json.addValue("time", (double) af_getsteady_ms());
        Json.addValue("url", mCurlhttpContext.location);
        Json.addValue("retryCount", mCurlhttpContext.reTryCount);
        Json.addValue("openCost", (int) mOpenTimeMS);
        Json.addValue("ip", mCurlhttpContext.ipStr);

        if (mCurlhttpContext.pConnection->response) {
            const std::string strResponse = mCurlhttpContext.pConnection->response;
            std::string theValue = DataSourceUtils::getPropertryOfResponse(strResponse, "EagleId:");

            if (!theValue.empty()) {
                Json.addValue("eagleID", theValue);
            }

            theValue = DataSourceUtils::getPropertryOfResponse(strResponse, "Via:");

            if (!theValue.empty()) {
                Json.addValue("cdnVia", theValue);
            }

            theValue = DataSourceUtils::getPropertryOfResponse(strResponse, "cdncip:");

            if (!theValue.empty()) {
                Json.addValue("cdncip", theValue);
            }

            theValue = DataSourceUtils::getPropertryOfResponse(strResponse, "cdnsip:");

            if (!theValue.empty()) {
                Json.addValue("cdnsip", theValue);
            }
        }

        typedef struct printInfo {
            const char *str;
            int key;
            int scale;
            int value;
        } printInfo;
        printInfo infos[] {
            {"dnsCost",       CURLINFO_NAMELOOKUP_TIME, 1000, 0},
            {"connectCost",   CURLINFO_CONNECT_TIME,    1000, 0},
            {"redirectCount", CURLINFO_REDIRECT_COUNT,  1,    0},
            {nullptr,         0,                        0,    0}
        };

        for (auto info : infos) {
            double val = info.value;

            if (info.str == nullptr) {
                break;
            }

            if (info.key >= 0) {
                curl_easy_getinfo(pConnection->http_handle, (CURLINFO) info.key, &val);
            }

            Json.addValue(info.str, (int) (val * info.scale));
        }

        mConnectInfo = Json.printJSON();
        AF_LOGD("connectInfo is %s\n", mConnectInfo.c_str());
    }

    bool CurlDataSource::probe(const string &path)
    {
        return CicadaUtils::startWith(path, {"http://", "https://"});
    }

    void CurlDataSource::Interrupt(bool interrupt)
    {
        IDataSource::Interrupt(interrupt);

        if (interrupt) {
            mCurlhttpContext.mSleepCondition.notify_one();
        }
    }

}
