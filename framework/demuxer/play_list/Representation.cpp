//
// Created by moqi on 2018/4/25.
//
#define LOG_TAG "Representation"

#include <utils/frame_work_log.h>
#include "Representation.h"
#include "utils/AFMediaType.h"

namespace Cicada {

//    void Representation::addSegment(segment *seg) {
//        mSegList.push_back(seg);
//
//    }

    Representation::~Representation()
    {
        delete mPSegList;
    }

    void Representation::setPlaylistUrl(const std::string &url)
    {
        mPlaylistUrl = url;
    }

    void Representation::setBaseUrl(const std::string &url)
    {
        mBaseUrl = url;
    }

    void Representation::setBandwidth(uint64_t bandwidth)
    {
        mBandWidth = bandwidth;
    }

    void Representation::setWidth(int width)
    {
        mWidth = width;
    }

    void Representation::setHeight(int height)
    {
        mHeight = height;
    }

    void Representation::print()
    {
//        AF_LOGD("Representation","url is %s\n"
//                "mBandWidth is %llu\n"
//                "res is %d x %d",mPlaylistUrl.c_str(),mBandWidth,mWidth,mHeight);
        if (mStreamType == STREAM_TYPE_AUDIO || mStreamType == STREAM_TYPE_SUB) {
            AF_LOGD("%s language is %s\n",
                    mStreamType == STREAM_TYPE_AUDIO ? "Audio" : "Subtitle",
                    mLang.c_str());
        } else if (mStreamType == STREAM_TYPE_VIDEO) {
            AF_LOGD("video size  is %d x %d with bandWidth %llu\n", mHeight, mWidth, mBandWidth);
        }

        if (mPSegList) {
            mPSegList->print();
        }
    }

    void Representation::SetSegmentList(SegmentList *segList)
    {
        mPSegList = segList;
    }

    const std::string &Representation::getBaseUrl()
    {
        return mBaseUrl;
    }

    playList *Representation::getPlaylist()
    {
        return mAdapt->getPeriod()->getPlayList();
    }

    SegmentList *Representation::GetSegmentList()
    {
        return mPSegList;
    }

    const std::string &Representation::getPlaylistUrl()
    {
        return mPlaylistUrl;
    }

    int Representation::getStreamInfo(int *width, int *height, uint64_t *bandwidth, std::string &language)
    {
        if (width) {
            *width = mWidth;
        }

        if (height) {
            *height = mHeight;
        }

        if (bandwidth) {
            *bandwidth = mBandWidth;
        }

        language = mLang;
        return 0;
    }

    AdaptationSet *Representation::getAdaptationSet()
    {
        return mAdapt;
    }
}