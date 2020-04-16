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

    class IDataSource : public OptionOwner {
    public:
        class Listener {
        public:
            enum NetWorkRetryStatus {
                NetWorkRetryStatusPending,
                NetWorkRetryStatusRetry,
//                NetWorkRetryStatusAbort,
            };

            virtual NetWorkRetryStatus onNetWorkRetry(int error) = 0;

            virtual void onNetWorkConnected()
            {
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

            std::string toString();
        };


        explicit IDataSource(std::string url);

        virtual ~IDataSource() = default;

        virtual int setRange(int64_t start, int64_t end);

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


    protected:
        std::atomic_bool mInterrupt{false};
        SourceConfig mConfig{};
        std::string mUri{};
        int64_t rangeStart{INT64_MIN};
        int64_t rangeEnd{INT64_MIN};

    };
}


#endif //FRAMEWORK_DATASOUTCE_H
