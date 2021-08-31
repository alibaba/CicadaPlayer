//
// Created by moqi on 2018/1/23.
//

#ifndef FRAMEWORK_DATASOUTCE_H
#define FRAMEWORK_DATASOUTCE_H

#include <string>
#include <base/OptionOwner.h>
#include <utils/CicadaType.h>
#include <vector>
#include <atomic>

namespace Cicada {

    enum {
        SEEK_SIZE = 0x10000,
    };

    struct mediaSegmentListEntry {
        std::string url;
        int64_t duration = 0;

        mediaSegmentListEntry(const std::string &_url, int64_t _duration) : url(_url), duration(_duration)
        {}
    };

    class IMediaInfoProvider {
    public:
        virtual int64_t estimateExclusiveEndPositionBytes(const std::string &url, int64_t timeMicSec, int64_t totalLength) = 0;
        virtual int64_t estimatePlayTimeMicSec(const std::string &url, int64_t filePosition, int64_t totalLength) = 0;
        /**
         * @return -1 unkown, 1 allow, 0 disallow
         */
        virtual int allowDataCache(const std::string &url) = 0;
        virtual std::vector<mediaSegmentListEntry> getSegmentList(int index) = 0;
    };

    class IDataSource : public OptionOwner {
    public:
        class Listener {
        public:
            enum NetWorkRetryStatus {
                NetWorkRetryStatusPending,
                NetWorkRetryStatusRetry,
//                NetWorkRetryStatusAbort,
            };

            enum bitStreamType {
                bitStreamTypeMedia,
            };

            virtual NetWorkRetryStatus onNetWorkRetry(int error) = 0;

            virtual void onNetWorkConnected()
            {
            }
            /*
             * return, whether stop report to other
             */
            virtual bool onNetWorkInPut(uint64_t size, bitStreamType type)
            {
                return false;
            }
        };

        enum speedLevel {
            speedLevel_mem,
            speedLevel_local,
            speedLevel_remote,
        };

        class SourceConfig {
        public:
            enum IpResolveType { IpResolveWhatEver, IpResolveV4, IpResolveV6 };

        public:
            int low_speed_limit{1};
            int low_speed_time_ms{15000};
//        int64_t max_time_ms{0};
            int connect_time_out_ms{15000};
            int so_rcv_size{0};
            std::string http_proxy = "";
            std::string refer = "";
            std::string userAgent = "";
            std::vector<std::string> customHeaders;
            Listener *listener = nullptr;
            IpResolveType resolveType{IpResolveWhatEver};
            bool enableLog{true};

            std::string toString();
        };

        const static uint64_t flag_report_speed = 1 << 0;

        explicit IDataSource(std::string url);

        virtual ~IDataSource() = default;

        virtual int setRange(int64_t start, int64_t end);

        virtual void setMediaInfoProvider(IMediaInfoProvider *provider);

        virtual void setPost(bool post, int64_t size, const uint8_t *data);

        virtual int Open(int flags) = 0;

        virtual int Open(const std::string &url);

        virtual void Close() = 0;

        virtual int64_t Seek(int64_t offset, int whence) = 0;

        virtual int Read(void *buf, size_t nbyte) = 0;

        virtual void Interrupt(bool interrupt);

        virtual std::string Get_error_info(int error);

        virtual void Set_config(SourceConfig &config);

        virtual void Get_config(SourceConfig &config);

        virtual std::string GetOption(const std::string &key);

        virtual const options *GetOption();

        virtual std::string GetUri();

        virtual speedLevel getSpeedLevel();

        virtual void setSegmentList(const std::vector<mediaSegmentListEntry> &segments)
        {}

        virtual void enableCache(const std::string &originUrl, bool enable)
        {}
        virtual uint64_t getFlags()
        {
            return 0;
        }


    protected:
        std::atomic_bool mInterrupt{false};
        SourceConfig mConfig{};
        /**
         * the main/first url
         */
        std::string mUri{};
        int64_t rangeStart{INT64_MIN};
        int64_t rangeEnd{INT64_MIN};

        bool mBPost{false};
        const uint8_t *mPostData{nullptr};
        int64_t mPostSize{0};

        IMediaInfoProvider *mMediaInfoProvider{nullptr};
    };
}


#endif //FRAMEWORK_DATASOUTCE_H
