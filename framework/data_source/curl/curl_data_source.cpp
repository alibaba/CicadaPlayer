//
// Created by moqi on 2018/1/25.
//
#define LOG_TAG "CurlDataSource"

#include "curl_data_source.h"
#include <pthread.h>

#include <utils/frame_work_log.h>
#include <utils/errors/framework_error.h>
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
//#include <openssl/opensslv.h>
#include <cstring>



// TODO: move to another file
#if defined(WIN32) || defined(WIN64)
    #define strcasecmp _stricmp
    #define strncasecmp _strnicmp
#endif /* Def WIN32 or Def WIN64 */

#define SOCKET_ERROR (-1)

#define MIN_SO_RCVBUF_SIZE 1024*64

//static pthread_mutex_t g_mutex; ///< we have nowhere to destroy this.
//static int g_lock_inited = 0;

static pthread_once_t once = PTHREAD_ONCE_INIT;
using namespace Cicada;

CurlDataSource CurlDataSource::se(0);
using std::string;

CURLConnection *CurlDataSource::initConnection()
{
    auto *pHandle = new CURLConnection(pConfig);
    pHandle->setSource(mLocation, headerList);
    return pHandle;
}

int CurlDataSource::curl_connect(CURLConnection *pConnection, int64_t filePos)
{
    int ret;
    char *location = nullptr;
    char *ipstr = nullptr;
    double length;
    long response;
    AF_LOGD("start connect %lld\n", filePos);
    pConnection->SetResume(filePos);
    pConnection->start();

    if ((ret = pConnection->FillBuffer(1)) < 0) {
        AF_LOGE("Connect, didn't get any data from stream.");
        return ret;
    }

    AF_LOGD("connected\n");

    if (CURLE_OK ==
            curl_easy_getinfo(pConnection->getCurlHandle(), CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length)) {
        if (length < 0) {
            length = 0.0;
        }

        if (length > 0.0) {
            mFileSize = pConnection->tell() + (int64_t) length;
            //AF_LOGE("file size is %lld\n",mFileSize);
        }

//        if (curlContext.fileSize == 0)
//            curlContext.hd->is_streamed = 1;
    }

    if (CURLE_OK ==
            curl_easy_getinfo(pConnection->getCurlHandle(), CURLINFO_EFFECTIVE_URL, &location)) {
        if (location) {
            mLocation = location;
        } else {
            mLocation = "";
        }
    }

    if (CURLE_OK == curl_easy_getinfo(pConnection->getCurlHandle(), CURLINFO_PRIMARY_IP, &ipstr)) {
        if (ipstr) {
            mIpStr = ipstr;
        } else {
            mIpStr = "";
        }
    }

    if (CURLE_OK ==
            curl_easy_getinfo(pConnection->getCurlHandle(), CURLINFO_RESPONSE_CODE, &response)) {
        AF_LOGI("CURLINFO_RESPONSE_CODE is %d", response);

        if (response >= 400) {
            return gen_framework_http_errno((int) response);
        }
    }

    return 0;
}


static void init_curl()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
#if (OPENSSL_VERSION_NUMBER < 0x10100000L)
//   openssl_thread_setup();
#endif
}

static void clean_curl()
{
    curl_global_cleanup();
#if (OPENSSL_VERSION_NUMBER < 0x10100000L)
    //openssl_thread_cleanup()
#endif
}

CurlDataSource::CurlDataSource(const string &url) : IDataSource(url)
{
    mFileSize = -1;
    mConnections = new std::vector<CURLConnection *>();
}

CurlDataSource::~CurlDataSource()
{
    AF_LOGI("!~CurlDataSource");

    if (mBDummy) {
        clean_curl();
        return;
    }

    Interrupt(true);
    Close();
}

int CurlDataSource::Open(int flags)
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
    mOpenTimeMS = af_gettime_relative() / 1000 - mOpenTimeMS;

    if (ret >= 0) {
        fillConnectInfo();
    }

    if (nullptr == mConnections) {
        mConnections = new std::vector<CURLConnection *>();
    }

    return ret;
}

int CurlDataSource::Open(const string &url)
{
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

    mOpenTimeMS = af_gettime_relative() / 1000;
    mPConnection->disconnect();
    bool isRTMP = url.compare(0, 7, "rtmp://") == 0;
    mLocation = (isRTMP ? (url + " live=1").c_str() : url.c_str());
    // only change url, don,t change share and resolve
    mPConnection->updateSource(mLocation);
    int ret = curl_connect(mPConnection, rangeStart != INT64_MIN ? rangeStart : 0);
    mOpenTimeMS = af_gettime_relative() / 1000 - mOpenTimeMS;

    if (ret >= 0) {
        fillConnectInfo();
    }

    closeConnections(false);
    mConnections = new std::vector<CURLConnection *>();
    return ret;
}

void CurlDataSource::Close()
{
    closeConnections(true);
}

void CurlDataSource::closeConnections(bool current)
{
    lock_guard<mutex> lock(mMutex);
    CURLConnection *deleteConnection = nullptr;
    vector<CURLConnection *> *pConnections = mConnections;
    mConnections = nullptr;

    if (current) {
        deleteConnection = mPConnection;
        mPConnection = nullptr;
    }

    if (deleteConnection) {
        AsyncJob::Instance()->addJob([deleteConnection] {
            delete deleteConnection;
        });
    }

    if (pConnections) {
        AsyncJob::Instance()->addJob([pConnections] {
            for (auto item = pConnections->begin(); item != pConnections->end();)
            {
                delete *item;
                item = pConnections->erase(item);
            }
            delete pConnections;
        });
    }
}

int64_t CurlDataSource::Seek(int64_t offset, int whence)
{
//    AF_LOGD("CurlDataSource::Seek position is %lld,when is %d", offset, whence);
    if (whence == SEEK_SIZE) {
        return mFileSize;
    } else if ((whence == SEEK_CUR && offset == 0) ||
               (whence == SEEK_SET && offset == mPConnection->tell())) {
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
        return -(EINVAL);
    }

    if (offset == mPConnection->tell()) {
        return offset;
    }

    if (offset > mFileSize) {
        return -1;
    }

    if (offset == mFileSize) {
    }

    //first seek in cache
    if (mPConnection->short_seek(offset) >= 0) {
        AF_LOGI("short seek ok\n");
        return offset;
    } else {
        AF_LOGI("short seek filed\n");
    }

    CURLConnection *con = nullptr;

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
        mConnections->push_back(mPConnection);

        if (mConnections->size() > max_connection) {
            CURLConnection *connection = mConnections->front();
            mConnections->erase(mConnections->begin());
            AsyncJob::Instance()->addJob([connection] {
                delete connection;
            });
        }

        mPConnection = con;
        AF_LOGW("short seek ok\n");
        return offset;
    } else {
        AF_LOGW("short seek filed\n");
    }

    int64_t ret = TrySeekByNewConnection(offset);
    return (ret >= 0) ? ret : -1;
}

int64_t CurlDataSource::TrySeekByNewConnection(int64_t offset)
{
    // try seek use a new connection
    CURLConnection *pConnection_s = initConnection();
    pConnection_s->setInterrupt(&mInterrupt);
    int ret = curl_connect(pConnection_s, offset);

    if (ret >= 0) {
        std::lock_guard<std::mutex> lock(mMutex);
        // try seek ok, use the new connection
        mConnections->push_back(mPConnection);

        if (mConnections->size() > max_connection) {
            CURLConnection *connection = mConnections->front();
            mConnections->erase(mConnections->begin());
            AsyncJob::Instance()->addJob([connection] {
                delete connection;
            });
        }

        mPConnection = pConnection_s;
        return offset;
    }

    // try seek faild, use the old connection
    AsyncJob::Instance()->addJob([pConnection_s] {
        delete pConnection_s;
    });
    return ret;
}

int CurlDataSource::Read(void *buf, size_t size)
{
    int ret = 0;

    if (rangeEnd != INT64_MIN) {
        size = std::min(size, (size_t) (rangeEnd - mPConnection->tell()));

        if (size == 0) {
            return 0;
        }
    }

    /* only request 1 byte, for truncated reads (only if not eof) */
    if (mFileSize <= 0 || mPConnection->tell() < mFileSize) {
        ret = mPConnection->FillBuffer(1);

        if (ret < 0) {
            return ret;
        }
    }

    return mPConnection->readBuffer(buf, size);
}

string CurlDataSource::GetOption(const string &key)
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

void CurlDataSource::fillConnectInfo()
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
            curl_easy_getinfo(mPConnection->getCurlHandle(), (CURLINFO) info.key, &val);
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
        mSleepCondition.notify_one();
    }
}

CurlDataSource::CurlDataSource(int dummy) : IDataSource("")
{
    mBDummy = true;
    init_curl();
    addPrototype(this);
}
