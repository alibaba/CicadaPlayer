//
// Created by moqi on 2018/1/25.
//

#ifndef FRAMEWORK_DATASOURCE_CURLSOURCE2_H
#define FRAMEWORK_DATASOURCE_CURLSOURCE2_H

#include "CURLConnection2.h"
#include "data_source/IDataSource.h"
#include "data_source/dataSourcePrototype.h"
#include "utils/ringBuffer.h"
#include <condition_variable>
#include <mutex>
#include <utils/globalNetWorkManager.h>

namespace Cicada {

    class CurlDataSource2 : public IDataSource,
                            private dataSourcePrototype,
                            private globalNetWorkManager::globalNetWorkManagerListener,
                            private CURLConnection2::CURLConnectionListener {
    public:
        static bool probe(const std::string &path);

        explicit CurlDataSource2(const std::string &url);

        ~CurlDataSource2() override;

        int Open(int flags) override;

        int Open(const std::string &url) override;

        void Close() final;

        int64_t Seek(int64_t offset, int whence) override;

        int Read(void *buf, size_t nbyte) override;

        std::string GetOption(const std::string &key) override;

        void Interrupt(bool interrupt) final;

        std::string GetUri() override;
        uint64_t getFlags() override
        {
            return flag_report_speed;
        }

    private:
        CURLConnection2 *initConnection();

        int64_t TrySeekByNewConnection(int64_t offset);

        void fillConnectInfo();

        int curl_connect(CURLConnection2 *pConnection, int64_t filePos);

    private:
        explicit CurlDataSource2(int dummy);

        IDataSource *clone(const std::string &uri) override
        {
            return new Cicada::CurlDataSource2(uri);
        }

        bool is_supported(const std::string &uri, int flags) override
        {
            if (flags != 0) {
                return false;
            }
            return probe(uri);
        }

        static CurlDataSource2 se;

        void closeConnections(bool current, bool forbidReuse);

        void OnReconnect() override;

        void onDNSResolved() override;

    private:
        const static int max_connection = 1;
        std::string mLocation;
        int64_t mFileSize = -1;
        CURLConnection2 *mPConnection = nullptr;
        Cicada::IDataSource::SourceConfig *pConfig = nullptr;
        int multisession = 0;
        std::string mIpStr;
        int reTryCount = 0;

        //add for custom http headers;
        struct curl_slist *headerList = nullptr;
        std::condition_variable mSleepCondition;
        int64_t mOpenTimeMS = 0;
        std::mutex mMutex;
        std::string mConnectInfo;
        bool mBDummy = false;
        std::vector<CURLConnection2 *> *mConnections{nullptr};
        std::atomic<bool> mNeedReconnect{false};
        CurlMulti *mMulti{nullptr};
        bool mDNSResolved{false};
        long mCurrentHttpVersion{0};
    };
}// namespace Cicada

#endif//FRAMEWORK_DATASOURCE_CURLSOURCE2_H
