//
// Created by moqi on 2018/1/25.
//
#define LOG_TAG "CurlDataSource2"

#include "curl_data_source2.h"
#include <pthread.h>

#include "CURLShareInstance.h"
#include "CurlMulti.h"
#include "data_source/DataSourceUtils.h"
#include "utils/CicadaJSON.h"
#include <thread>
#include <utils/errors/framework_error.h>
#include <utils/frame_work_log.h>
#include <utils/property.h>
#include <utils/timer.h>

#ifdef WIN32
#include <winsock2.h>
#endif

#include <algorithm>
#include <cerrno>
#include <utils/CicadaUtils.h>
//#include <openssl/opensslv.h>
#include <cassert>
#include <cstring>


// TODO: move to another file
#if defined(WIN32) || defined(WIN64)
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif /* Def WIN32 or Def WIN64 */

#define SOCKET_ERROR (-1)

#define MIN_SO_RCVBUF_SIZE 1024 * 64

#define CURL_LOGD(...)                                                                                                                     \
    do {                                                                                                                                   \
        if (mConfig.enableLog) __log_print(AF_LOG_LEVEL_DEBUG, LOG_TAG, __VA_ARGS__);                                                      \
    } while (0)

//static pthread_mutex_t g_mutex; ///< we have nowhere to destroy this.
//static int g_lock_inited = 0;

static pthread_once_t once = PTHREAD_ONCE_INIT;
using namespace Cicada;

CurlDataSource2 CurlDataSource2::se(0);
using std::string;

#define USE_MULTI 0
#define SEEK_USE_NEW_CONNECTION 0

CURLConnection2 *CurlDataSource2::initConnection()
{
    auto *pHandle = new CURLConnection2(&mConfig, mMulti, nullptr);
    pHandle->setSSLBackEnd(CURLShareInstance::Instance()->getSslbakcend());
    pHandle->setSource(mLocation, headerList);
    pHandle->setPost(mBPost, mPostSize, mPostData);
    return pHandle;
}

int CurlDataSource2::curl_connect(CURLConnection2 *pConnection, int64_t filePos)
{
    int ret;
    char *location = nullptr;
    char *ipstr = nullptr;
    double length;
    long response;
    CURL_LOGD("start connect %lld\n", filePos);
    pConnection->SetResume(filePos);
    pConnection->addToMulti();

    if ((ret = pConnection->FillBuffer(1, *mMulti, mNeedReconnect)) < 0) {
        AF_LOGE("Connect, didn't get any data from stream.");
        return ret;
    }

    CURL_LOGD("connected\n");

    if (CURLE_OK == curl_easy_getinfo(pConnection->getCurlHandle(), CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length)) {
        if (length < 0) {
            length = 0.0;
        }

        if (length > 0.0) {
            mFileSize = pConnection->tell() + (int64_t) length;
            //AF_LOGE("file size is %lld\n",mFileSize);
        } else {
            mFileSize = 0;
        }

        //        if (curlContext.fileSize == 0)
        //            curlContext.hd->is_streamed = 1;
    }

    if (CURLE_OK == curl_easy_getinfo(pConnection->getCurlHandle(), CURLINFO_EFFECTIVE_URL, &location)) {
        if (location) {
            mLocation = location;
        }
    }

    if (CURLE_OK == curl_easy_getinfo(pConnection->getCurlHandle(), CURLINFO_PRIMARY_IP, &ipstr)) {
        if (ipstr) {
            mIpStr = ipstr;
        } else {
            mIpStr = "";
        }
    }

    if (CURLE_OK == curl_easy_getinfo(pConnection->getCurlHandle(), CURLINFO_RESPONSE_CODE, &response)) {
        CURL_LOGD("CURLINFO_RESPONSE_CODE is %d", response);

        if (response >= 400) {
            return gen_framework_http_errno((int) response);
        }
    }
    if (CURLE_OK == curl_easy_getinfo(pConnection->getCurlHandle(), CURLINFO_HTTP_VERSION, &mCurrentHttpVersion)) {
        AF_LOGD("current http version is %d\n", mCurrentHttpVersion);
    }

    return 0;
}

CurlDataSource2::CurlDataSource2(const string &url) : IDataSource(url)
{
    globalNetWorkManager::getGlobalNetWorkManager()->addListener(this);
    mFileSize = -1;
    mConnections = new std::vector<CURLConnection2 *>();
    mMulti = CurlMultiManager::getCurlMulti();
}

CurlDataSource2::~CurlDataSource2()
{
    AF_LOGI("CurlDataSource2");
    if (mBDummy) {
        return;
    }
    if (globalNetWorkManager::getGlobalNetWorkManager()) {
        globalNetWorkManager::getGlobalNetWorkManager()->removeListener(this);
    }
    Interrupt(true);
    Close();
    delete mConnections;
    if (headerList) {
        curl_slist_free_all(headerList);
        headerList = nullptr;
    }
    AF_LOGI("~!CurlDataSource2");
}

int CurlDataSource2::Open(int flags)
{
    // TODO: deal with ret
    mOpenTimeMS = af_gettime_relative() / 1000;
    bool isRTMP = mUri.compare(0, 7, "rtmp://") == 0;
    mLocation = (isRTMP ? (mUri + " live=1").c_str() : mUri.c_str());
    pConfig = &mConfig;

    if (headerList) {
        curl_slist_free_all(headerList);
        headerList = nullptr;
    }

    if (getProperty("ro.network.http.globeHeader")) {
        headerList = curl_slist_append(headerList, getProperty("ro.network.http.globeHeader"));
    }

    std::vector<std::string> &customHeaders = mConfig.customHeaders;

    for (string &item : customHeaders) {
        if (!item.empty()) {
            headerList = curl_slist_append(headerList, item.c_str());
        }
    }

    if (pConfig->so_rcv_size >= MIN_SO_RCVBUF_SIZE) {
        pConfig->so_rcv_size = pConfig->so_rcv_size >> 12;
        pConfig->so_rcv_size = pConfig->so_rcv_size << 12;
        AF_LOGI("so_rcv_size is %d\n", pConfig->so_rcv_size);
    } else if (pConfig->so_rcv_size > 0) {
        AF_LOGI("so_rcv_size too small\n");
        pConfig->so_rcv_size = 0;
    }

    {
        std::lock_guard<std::mutex> lock(mMutex);
        mPConnection = initConnection();
        mPConnection->setInterrupt(&mInterrupt);
    }

    int ret = curl_connect(mPConnection, rangeStart != INT64_MIN ? rangeStart : 0);
    if (mNeedReconnect) {
        curl_easy_setopt(mPConnection->getCurlHandle(), CURLOPT_FORBID_REUSE, 1);
        return Open(mUri);
    }
    mOpenTimeMS = af_gettime_relative() / 1000 - mOpenTimeMS;

    if (ret >= 0) {
        fillConnectInfo();
    }

    if (nullptr == mConnections) {
        mConnections = new std::vector<CURLConnection2 *>();
    }

    return ret;
}

int CurlDataSource2::Open(const string &url)
{
    if (mNeedReconnect) {
        closeConnections(false);
        mNeedReconnect = false;
    }
    if (mPConnection == nullptr) {
        mUri = url;
        return Open(0);
    }

    if (mUri == url) {
        if (rangeStart != INT64_MIN) {
            Seek(rangeStart, SEEK_SET);
            return 0;
        }
    }
#if 1
    closeConnections(true);
    mUri = url;
    return Open(0);
#elif
    mOpenTimeMS = af_gettime_relative() / 1000;

    mPConnection->removeFormMulti();
    mPConnection->reset();
    bool isRTMP = url.compare(0, 7, "rtmp://") == 0;
    mLocation = (isRTMP ? (url + " live=1").c_str() : url.c_str());
    // only change url, don,t change share and resolve
    mPConnection->updateSource(mLocation);

    if (headerList) {
        curl_slist_free_all(headerList);
        headerList = nullptr;
    }

    std::vector<std::string> &customHeaders = mConfig.customHeaders;

    for (string &item : customHeaders) {
        if (!item.empty()) {
            headerList = curl_slist_append(headerList, item.c_str());
        }
    }

    mPConnection->updateHeaderList(headerList);
    mPConnection->setPost(mBPost, mPostSize, mPostData);

    int ret = curl_connect(mPConnection, rangeStart != INT64_MIN ? rangeStart : 0);
    mOpenTimeMS = af_gettime_relative() / 1000 - mOpenTimeMS;

    if (ret >= 0) {
        fillConnectInfo();
    }

    closeConnections(false);
    return ret;
#endif
}

void CurlDataSource2::Close()
{
    if (mPConnection) {
        mPConnection->disableListener();
    }
    if (mConnections) {
        for (auto item : *mConnections) {
            item->disableListener();
        }
    }
    closeConnections(true);
}

void CurlDataSource2::closeConnections(bool current)
{
    lock_guard<mutex> lock(mMutex);
    if (current && mPConnection) {
        mPConnection->deleteFormMulti();
        mPConnection = nullptr;
    }

    if (mConnections) {
        for (auto item = mConnections->begin(); item != mConnections->end();) {
            (*item)->deleteFormMulti();
            item = mConnections->erase(item);
        }
    }
}

int64_t CurlDataSource2::Seek(int64_t offset, int whence)
{
    //    CURL_LOGD("CurlDataSource2::Seek position is %lld,when is %d", offset, whence);
    if (!mPConnection) {
        return -(ESPIPE);
    }
    if (whence == SEEK_SIZE) {
        return mFileSize;
    } else if (((whence == SEEK_CUR && offset == 0) || (whence == SEEK_SET && offset == mPConnection->tell())) &&
               !mPConnection->needReconnect()) {
        return mPConnection->tell();
    } else if ((mFileSize <= 0 && whence == SEEK_END) /*|| h->is_streamed*/) {
        return FRAMEWORK_ERR(ENOSYS);
    }

    if (whence == SEEK_CUR) {
        offset += mPConnection->tell();
    } else if (whence == SEEK_END) {
        offset += mFileSize;
    } else if (whence != SEEK_SET) {
        return FRAMEWORK_ERR(EINVAL);
    }

    if (offset < 0) {
        return -(ESPIPE);
    }

    if (offset == mPConnection->tell() && !mPConnection->needReconnect()) {
        return offset;
    }

    /* do not try to make a new connection if seeking past the end of the file */
    if (rangeEnd != INT64_MIN || mFileSize > 0) {
        uint64_t end_pos = rangeEnd != INT64_MIN ? rangeEnd : mFileSize;
        if (offset >= end_pos) {
            mPConnection->SetResume(offset);
            return offset;
        }
    }

    if (offset == mFileSize) {
    }

    //first seek in cache
    if (!mNeedReconnect) {
        if (mPConnection->short_seek(offset) >= 0) {
            AF_LOGI("short seek ok\n");
            return offset;
        } else {
            AF_LOGI("short seek failed\n");
        }
    } else {
        curl_easy_setopt(mPConnection->getCurlHandle(), CURLOPT_FORBID_REUSE, 1);
        closeConnections(true);
    }
#if USE_MULTI
    CURLConnection2 *con = nullptr;
    for (auto item = mConnections->begin(); item != mConnections->end();) {
        if ((*(item))->short_seek(offset) >= 0) {
            con = *item;
            item = mConnections->erase(item);
            break;
        } else {
            ++item;
        }
    }

    if (con) {
        mPConnection->pause(true);
        mPConnection->removeFormMulti();
        mConnections->push_back(mPConnection);

        if (mConnections->size() > max_connection) {
            CURLConnection2 *connection = mConnections->front();
            mConnections->erase(mConnections->begin());
            mMulti->deleteHandle(connection);
            connection->deleteFormMulti();
        }
        mPConnection = con;
        mPConnection->addToMulti();
        mPConnection->pause(false);
        AF_LOGW("short seek ok\n");
        return offset;
    } else {
        AF_LOGW("short seek failed\n");
    }
#endif
    int64_t ret = TrySeekByNewConnection(offset);
    mNeedReconnect = false;
    return ret;
}

int64_t CurlDataSource2::TrySeekByNewConnection(int64_t offset)
{
#if USE_MULTI
    // try seek use a new connection
    if (mPConnection) {
        mPConnection->removeFormMulti();
    }
    CURLConnection2 *pConnection_s = initConnection();
    if (mNeedReconnect || SEEK_USE_NEW_CONNECTION) {
        pConnection_s->applyReconnect(true);
    }
    pConnection_s->setInterrupt(&mInterrupt);
    int ret = curl_connect(pConnection_s, offset);

    if (ret >= 0) {
        std::lock_guard<std::mutex> lock(mMutex);
        // try seek ok, use the new connection
        mConnections->push_back(mPConnection);

        if (mConnections->size() > max_connection) {
            CURLConnection2 *connection = mConnections->front();
            mConnections->erase(mConnections->begin());
            connection->deleteFormMulti();
        }

        mPConnection = pConnection_s;
        return offset;
    }

    // try seek faild, use the old connection
    pConnection_s->deleteFormMulti();
    return ret;
#else
    if (mPConnection) {
        mPConnection->deleteFormMulti();
    }
    CURLConnection2 *pConnection_s = initConnection();
    pConnection_s->setInterrupt(&mInterrupt);
    if (mNeedReconnect || SEEK_USE_NEW_CONNECTION) {
        pConnection_s->applyReconnect(true);
    }
    int ret = curl_connect(pConnection_s, offset);
    mPConnection = pConnection_s;
    return offset;
#endif
}

int CurlDataSource2::Read(void *buf, size_t size)
{
    int ret = 0;

    if (rangeEnd != INT64_MIN || mFileSize > 0) {
        /*
        * avoid read after seek to end
        */

        int64_t end = mFileSize;
        if (rangeEnd > 0) {
            end = rangeEnd;
        }
        end = std::min(mFileSize, end);

        if (end > 0) {
            size = std::min(size, (size_t) (end - mPConnection->tell()));

            if (size <= 0) {
                return 0;
            }
        }
    }

    /* only request 1 byte, for truncated reads (only if not eof) */
    if (mFileSize <= 0 || mPConnection->tell() < mFileSize) {
        ret = mPConnection->FillBuffer(1, *mMulti, mNeedReconnect);
        if (mNeedReconnect) {
            curl_easy_setopt(mPConnection->getCurlHandle(), CURLOPT_FORBID_REUSE, 1);
            closeConnections(false);
            mPConnection->setReconnect(true);
            int64_t seek = mPConnection->tell();
            int64_t seekRet = Seek(seek, SEEK_SET);
            assert(seekRet == seek);
        }

        if (ret < 0) {
            AF_LOGE("CurlDataSource2::Read ret=%d", ret);
            return ret;
        }
    }
    ret = mPConnection->readBuffer(buf, size);
    if (ret < 0) {
        AF_LOGE("CurlDataSource2::Read ret=%d", ret);
    }
    return ret;
}

string CurlDataSource2::GetOption(const string &key)
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (key == "responseInfo") {
        if (mPConnection && mPConnection->getResponse()) {
            CicadaJSONItem Json;
            Json.addValue("response", mPConnection->getResponse());
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

void CurlDataSource2::fillConnectInfo()
{
    CicadaJSONItem Json;
    Json.addValue("time", (double) af_getsteady_ms());
    Json.addValue("url", mLocation);
    Json.addValue("retryCount", reTryCount);
    Json.addValue("openCost", (int) mOpenTimeMS);
    Json.addValue("ip", mIpStr);

    if (mPConnection->getResponse()) {
        const std::string strResponse = mPConnection->getResponse();
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

    enum valueType {
        valueTypeDouble,
        valueTypeLong,
    };
    typedef struct printInfo {
        const char *str;
        int key;
        valueType type;
        int scale;
        int value;
    } printInfo;
    printInfo infos[]{{"dnsCost", CURLINFO_NAMELOOKUP_TIME, valueTypeDouble, 1000, 0},
                      {"connectCost", CURLINFO_CONNECT_TIME, valueTypeDouble, 1000, 0},
                      {"redirectCount", CURLINFO_REDIRECT_COUNT, valueTypeLong, 1, 0},
                      {"pv", CURLINFO_HTTP_VERSION, valueTypeLong, 1, 0},
                      {nullptr, 0, valueTypeDouble, 0}};

    for (auto &info : infos) {
        if (info.str == nullptr || info.key <= 0) {
            break;
        }
        switch (info.type) {
            case valueTypeDouble: {
                double val = info.value;
                curl_easy_getinfo(mPConnection->getCurlHandle(), (CURLINFO) info.key, &val);
                Json.addValue(info.str, (int) (val * info.scale));
                break;
            }
            case valueTypeLong: {
                long val = info.value;
                curl_easy_getinfo(mPConnection->getCurlHandle(), (CURLINFO) info.key, &val);
                Json.addValue(info.str, (int) (val * info.scale));
                break;
            }
        }
    }

    mConnectInfo = Json.printJSON();
    CURL_LOGD("connectInfo is %s\n", mConnectInfo.c_str());
}

bool CurlDataSource2::probe(const string &path)
{
    if (globalSettings::getSetting().getProperty("protected.network.http.http2") != "ON") {
        return false;
    }
    return CicadaUtils::startWith(path, {"http://", "https://"});
}

void CurlDataSource2::Interrupt(bool interrupt)
{
    IDataSource::Interrupt(interrupt);

    if (interrupt) {
        mSleepCondition.notify_one();
    }
}

CurlDataSource2::CurlDataSource2(int dummy) : IDataSource("")
{
    mBDummy = true;
    addPrototype(this);
}

std::string CurlDataSource2::GetUri()
{
    return mLocation;
}

void CurlDataSource2::OnReconnect()
{
    mNeedReconnect = true;
}
void CurlDataSource2::onDNSResolved()
{
    mDNSResolved = true;
}
