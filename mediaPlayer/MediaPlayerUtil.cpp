//
//  MediaPlayerUtil.cpp
//  ApsaraPlayer
//
//  Created by huang_jiafa on 2010/01/30.
//  Copyright (c) 2019 Aliyun. All rights reserved.
//
#define LOG_TAG "MeidaPlayerUtil"
#include "utils/frame_work_log.h"
#include "MediaPlayerUtil.h"
#include "utils/timer.h"
#include "utils/CicadaJSON.h"

using namespace std;

namespace Cicada {

    void MediaPlayerUtil::notifyPlayerLoop(int64_t time)
    {
        mLoopIndex++;

        if (mLastLoopTime == 0) {
            mLastLoopTime = time;
        } else {
            float timeS = float(time - mLastLoopTime) / 1000000;

            if (timeS > 1.0) {
                AF_LOGD("loop index is %f\n", (float) mLoopIndex / timeS);
                mLoopIndex = 0;
                mLastLoopTime = time;
            }
        }
    }

    void MediaPlayerUtil::render(int64_t pts)
    {
        ++mTotalRenderCount;

        if (1 == mTotalRenderCount) {
            mFirstRenderTime = af_getsteady_ms();
            mLastRenderTime = af_getsteady_ms();
            mLastRenderCount = 1;
        } else {
            int64_t diff = af_getsteady_ms() - mLastRenderTime;

            if (1000 <= diff) {
                mVideoRenderFps = (float) (mTotalRenderCount - mLastRenderCount) * 1000 / diff;
                AF_LOGD("KPI test total fps:%0.1f, Current FPS:%0.1f",
                        (float) (mTotalRenderCount - 1) * 1000 / (af_getsteady_ms() - mFirstRenderTime),
                        mVideoRenderFps);
                mLastRenderCount = mTotalRenderCount;
                mLastRenderTime = af_getsteady_ms();
            }
        }
    }

    void MediaPlayerUtil::reset()
    {
        int64_t diff = (af_getsteady_ms() - mFirstRenderTime);

        if ((0 < diff) && (0 < mFirstRenderTime)) {
            AF_LOGI("KPI test finish: total fps:%0.1f", (float) (mTotalRenderCount - 1) * 1000 / diff);
        }

        mTotalRenderCount = 0;
        mLastRenderCount = 0;
        mFirstRenderTime = 0;
        mLastRenderTime = 0;
        mLastLoopTime = 0;
        mLoopIndex = 0;
        mVideoRenderFps = 0;
    }

    void MediaPlayerUtil::getPropertyJSONStr(const std::string &name, CicadaJSONArray &array, bool isArray,
            std::deque<StreamInfo *> &streamInfoQueue, demuxer_service *service)
    {
        if (nullptr == service) {
            return;
        }

        for (auto &it : streamInfoQueue) {
            std::string str = service->GetProperty(it->streamIndex, name);

            if (str.empty()) {
                continue;
            }

            if (isArray) {
                CicadaJSONArray subArray(str);

                for (int i = 0; i < subArray.getSize(); ++i) {
                    CicadaJSONItem &tempItem = subArray.getItem(i);
                    addPropertyType(tempItem, it->type);
                    array.addJSON(tempItem);
                }
            } else {
                CicadaJSONItem loopItem(str);
                addPropertyType(loopItem, it->type);
                array.addJSON(loopItem);
            }
        }
    }

    void MediaPlayerUtil::addPropertyType(CicadaJSONItem &item, StreamType type)
    {
        switch (type) {
            case ST_TYPE_VIDEO:
                item.addValue("type", "video");
                break;

            case ST_TYPE_AUDIO:
                item.addValue("type", "audio");
                break;

            case ST_TYPE_SUB:
                item.addValue("type", "subtitle");
                break;

            default:
                item.addValue("type", "unknown");
                break;
        }
    }

    void MediaPlayerUtil::addURLProperty(const std::string &name, CicadaJSONArray &array, IDataSource *dataSource)
    {
        if (dataSource) {
            string str = dataSource->GetOption(name);

            if (str.empty()) {
                return;
            }

            CicadaJSONItem item(str);
            item.addValue("type", "url");
            array.addJSON(item);
        }
    }

    void MediaPlayerUtil::notifyRead(enum readEvent event)
    {
        switch (event) {
            case readEvent_Again:
                mReadAgainIndex++;
                break;

            case readEvent_Got:
                mReadGotIndex++;
                break;

            case readEvent_timeOut:
                mReadTimeOutIndex++;
                break;

            case readEvent_Loop:
                mReadLoopIndex++;
                break;
        }

        int64_t time = af_gettime_relative();

        if (mLastReadTime == 0) {
            mLastReadTime = time;
        } else {
            float timeS = float(time - mLastReadTime) / 1000000;

            if (timeS > 1.0) {
                AF_LOGD("mReadLoopIndex is \t %f\n", (float) mReadLoopIndex / timeS);
                AF_LOGD("mReadAgainIndex is\t %f\n", (float) mReadAgainIndex / timeS);
                AF_LOGD("mReadGotIndex is\t %f\n", (float) mReadGotIndex / timeS);
                AF_LOGD("mReadTimeOutIndex\t is %f\n", (float) mReadTimeOutIndex / timeS);
                AF_LOGD("\n");
                mLastReadTime = time;
                mReadLoopIndex = mReadAgainIndex = mReadGotIndex = mReadTimeOutIndex = 0;
            }
        }
    }

}
