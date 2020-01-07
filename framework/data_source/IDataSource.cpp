//
// Created by moqi on 2018/11/9.
//

#include "IDataSource.h"
#include <utils/frame_work_log.h>

#include <utility>
#include <cerrno>
#include <utils/CicadaUtils.h>
#include <unistd.h>

using std::string;

namespace Cicada {
    IDataSource::IDataSource(string url) : mUri(std::move(url))
    {
    }

    string IDataSource::Get_error_info(int error)
    {
        return string("Unknown error");;
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

}
