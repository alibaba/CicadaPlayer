//
// Created by moqi on 2018/11/9.
//
#define LOG_TAG "IDataSource"

#include "IDataSource.h"
#include <utils/frame_work_log.h>

#include <utility>
#include <cerrno>
#include <utils/CicadaUtils.h>
#include <unistd.h>
#include <utils/CicadaJSON.h>

using std::string;

namespace Cicada {
    IDataSource::IDataSource(string url) : mUri(std::move(url))
    {
    }

    string IDataSource::Get_error_info(int error)
    {
        return string("Unknown error");
    }

    string IDataSource::GetUri()
    {
        return mUri;
    }

    void IDataSource::Set_config(SourceConfig &config)
    {
        mConfig = config;
    };

    void IDataSource::Get_config(SourceConfig &config)
    {
        config = mConfig;
    }

    void IDataSource::Interrupt(bool interrupt)
    {
        mInterrupt = interrupt;
        AF_LOGD("IDataSource interrupt is %d", interrupt);
    }

    string IDataSource::GetOption(const string &key)
    {
        return std::string();
    }

    int IDataSource::Open(const string &url)
    {
        Close();
        mUri = url;
        return Open(0);
    }

    IDataSource::speedLevel IDataSource::getSpeedLevel()
    {
        if (CicadaUtils::startWith(mUri, {"http://", "https://", "rtmp://"})) {
            return speedLevel_remote;
        }

        if (access(mUri.c_str(), 0) == 0) {
            return speedLevel_local;
        }

        return speedLevel_remote;
    }

    const options *IDataSource::GetOption()
    {
        return mOpts;
    }

    int IDataSource::setRange(int64_t start, int64_t end)
    {
        rangeStart = start;
        rangeEnd = end;
        return 0;
    }

    std::string IDataSource::SourceConfig::toString()
    {
        CicadaJSONItem item{};
        item.addValue("low_speed_limit", low_speed_limit);
        item.addValue("low_speed_time_ms", low_speed_time_ms);
        item.addValue("connect_time_out_ms", connect_time_out_ms);
        item.addValue("so_rcv_size", so_rcv_size);
        item.addValue("http_proxy", http_proxy);
        item.addValue("refer", refer);
        item.addValue("userAgent", userAgent);
        std::string headerStr{};

        for (std::string &headerItem : customHeaders) {
            if (!headerItem.empty()) {
                headerStr.append(headerItem).append(";");
            }
        }

        item.addValue("customHeaders", headerStr);
        return item.printJSON();
    }
}
