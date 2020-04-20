//
// Created by moqi on 2018/1/24.
//
#include "ffmpeg_data_source.h"

#define LOG_TAG "ffmpegDataSource"

#include <utils/frame_work_log.h>
#include <utils/errors/framework_error.h>
#include <utils/ffmpeg_utils.h>

extern "C" {
#include <libavutil/error.h>
#include <libavutil/common.h>
}
using std::string;
namespace Cicada {

    ffmpegDataSource ffmpegDataSource::se(0);

    ffmpegDataSource::ffmpegDataSource(const string &url) : IDataSource(url)
    {
        mInterruptCB.callback = check_interrupt;
        mInterruptCB.opaque = this;
        ffmpeg_init();
    }

    ffmpegDataSource::~ffmpegDataSource()
    {
        Close();
    }

    int ffmpegDataSource::Open(int flags)
    {
        AVDictionary *format_opts = nullptr;
        av_dict_set_int(&format_opts, "rw_timeout", mConfig.low_speed_time_ms * 1000, 0);
        int ret = ffurl_open(&mPuc, mUri.c_str(), AVIO_FLAG_READ | AVIO_FLAG_NONBLOCK, &mInterruptCB, &format_opts);

        if (ret == AVERROR_PROTOCOL_NOT_FOUND) {
            ret = FRAMEWORK_ERR_PROTOCOL_NOT_SUPPORT;
        }
        if (format_opts) {
            av_dict_free(&format_opts);
        }

        if (rangeStart != INT64_MIN) {
            ffurl_seek(mPuc, (int64_t) rangeStart, SEEK_SET);
        }

        return ret;
    }

    void ffmpegDataSource::Close()
    {
        if (mPuc) {
            ffurl_closep(&mPuc);
            mPuc = nullptr;
        }
    }

    int64_t ffmpegDataSource::Seek(int64_t offset, int whence)
    {
        if (whence == SEEK_SIZE) {
            whence = AVSEEK_SIZE;
        }

        if (mPuc) {
            return ffurl_seek(mPuc, offset, whence);
        }

        return -EINVAL;
    }

    int ffmpegDataSource::Read(void *buf, size_t nbyte)
    {
        if (rangeEnd != INT64_MIN) {
            nbyte = std::min(nbyte, (size_t) (rangeEnd - Seek(0, SEEK_CUR)));

            if (nbyte == 0) {
                return 0;
            }
        }

        int ret = ffurl_read(mPuc, (unsigned char *) buf, nbyte);

        if (ret == AVERROR_EOF) {
            ret = 0;
        }

        return ret;
    }

    void ffmpegDataSource::Interrupt(bool interrupt)
    {
        mInterrupted = interrupt;
    }

    string ffmpegDataSource::Get_error_info(int error)
    {
        int ret = av_strerror(error, mErrorMsg, sizeof(mErrorMsg));

        if (ret < 0) {
            return "";
        }

        return (const char *) mErrorMsg;
    }

    string ffmpegDataSource::GetOption(const string &key)
    {
        //TODO
//        std::lock_guard<std::mutex> lock(mMutex);
//        if (key == "responseInfo") {
//                CicadaJSONItem Json;
//                Json.addValue("response", response);
//                return Json.printJSON();
//        }
//
//        if (key == "connectInfo") {
//        }
        return IDataSource::GetOption(key);
    }

    int ffmpegDataSource::check_interrupt(void *pHandle)
    {
        auto *source = (ffmpegDataSource *) pHandle;
        return source->mInterrupted;
    }
}

