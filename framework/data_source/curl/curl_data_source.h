//
// Created by moqi on 2018/1/25.
//

#ifndef FRAMEWORK_DATASOURCE_CURLSOURCE_H
#define FRAMEWORK_DATASOURCE_CURLSOURCE_H

#include <curl/multi.h>
#include "data_source/IDataSource.h"
#include "utils/ringBuffer.h"
#include <mutex>
#include <condition_variable>
#include "data_source/dataSourcePrototype.h"

namespace Cicada{

    class CURLConnection {
    public:
        CURL *http_handle = nullptr;
        std::string uri;

        RingBuffer *pRbuf = nullptr;
        char *pOverflowBuffer = nullptr;
        uint32_t overflowSize = 0;
        int sendRange = 0;
        int64_t filePos = 0;
        int seekable = 0;
        int m_bFirstLoop = 0;
        std::atomic_bool  *pInterrupted = nullptr;

        int so_rcv_size = 0;
        char *response = nullptr;
        int responseSize = 0;
        std::string outHeader = "";
        std::string inHeader = "";
        CURLM *multi_handle = nullptr;
        int still_running = 0;
    };

    class CURLHTTPContext {
    public:
        ~CURLHTTPContext();

        std::string location;
        int64_t fileSize = -1;
        CURLConnection *pConnection = nullptr;
        Cicada::IDataSource::SourceConfig *pConfig = nullptr;

        //   pthread_t multi_thread_id;
        int multisession = 0;
        std::string ipStr;
        int reTryCount = 0;

        //add for custom http headers;
        struct curl_slist *headerList = NULL;
        struct curl_slist *reSolveList = NULL;

        std::mutex mSleepMutex;
        std::condition_variable mSleepCondition;
    };

    class CurlDataSource : public IDataSource, private dataSourcePrototype {
    public:
        static bool probe(const std::string &path);

        explicit CurlDataSource(const  std::string &url);

        ~CurlDataSource() override;

        int Open(int flags) override;

        int Open(const std::string &url) override;

        void Close() override;

        int64_t Seek(int64_t offset, int whence) override;

        int Read(void *buf, size_t nbyte) override;

        std::string GetOption(const std::string &key) override;

        void Interrupt(bool interrupt) override;

    private:

        int64_t TrySeekByNewConnection(int64_t offset);

        void fillConnectInfo();

        int curl_connect(CURLHTTPContext &curlContext, CURLConnection *pConnection);

        int curl_easy_seek(CURLHTTPContext &curlContext, int64_t off);

        int FillBuffer(CURLHTTPContext &curlContext, CURLConnection *pConnection, uint32_t want);

        int getErrorCode(const CURLcode &CURLResult) const;

    private:
        explicit CurlDataSource(int dummy):IDataSource("")
        {
            addPrototype(this);
        }

        IDataSource *clone(const std::string &uri) override
        {
            return new Cicada::CurlDataSource(uri);
        }

        bool is_supported(const std::string &uri) override
        {
            return probe(uri);
        }

        static CurlDataSource se;

    private:
        // return -2 to retry

        CURLHTTPContext mCurlhttpContext;

        int64_t mOpenTimeMS = 0;

        std::mutex mMutex;
        std::string mConnectInfo;
    };
}

#endif //FRAMEWORK_DATASOURCE_CURLSOURCE_H
