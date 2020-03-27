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
#include "CURLConnection.h"

namespace Cicada {

    class CurlDataSource : public IDataSource, private dataSourcePrototype {
    public:
        static bool probe(const std::string &path);

        explicit CurlDataSource(const std::string &url);

        ~CurlDataSource() override;

        int Open(int flags) override;

        int Open(const std::string &url) override;

        void Close() override;

        int64_t Seek(int64_t offset, int whence) override;

        int Read(void *buf, size_t nbyte) override;

        std::string GetOption(const std::string &key) override;

        void Interrupt(bool interrupt) override;

    private:

        CURLConnection *initConnection();

        int64_t TrySeekByNewConnection(int64_t offset);

        void fillConnectInfo();

        int curl_connect(CURLConnection *pConnection, int64_t filePos);

    private:
        explicit CurlDataSource(int dummy);

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
        const static int max_connection = 1;
        std::string mLocation;
        int64_t mFileSize = -1;
        CURLConnection *mPConnection = nullptr;
        Cicada::IDataSource::SourceConfig *pConfig = nullptr;
        //   pthread_t multi_thread_id;
        int multisession = 0;
        std::string mIpStr;
        int reTryCount = 0;

        //add for custom http headers;
        struct curl_slist *headerList = nullptr;
        std::mutex mSleepMutex;
        std::condition_variable mSleepCondition;
        int64_t mOpenTimeMS = 0;
        std::mutex mMutex;
        std::string mConnectInfo;
        bool mBDummy = false;
        std::vector<CURLConnection *>* mConnections {nullptr};

        void closeConnections(bool current);
    };
}

#endif //FRAMEWORK_DATASOURCE_CURLSOURCE_H
