//
//  AbrAlgoStrategy.cpp
//  apsara_player
//
//  Created by shiping.csp on 2018/11/1.
//
#define LOG_TAG "AbrBufferAlgoStrategy"
#include "AbrBufferAlgoStrategy.h"
#include "AbrBufferRefererData.h"
#include <utils/frame_work_log.h>
#include <utils/timer.h>

#define LOWER_SWITCH_VALUE_MS (15*1000)
#define UPPER_SWITCH_VALUE_MS (30*1000)
#define MAX_BUFFER_STATICS_SIZE 10

AbrBufferAlgoStrategy::AbrBufferAlgoStrategy(std::function<void(int)> func)
    : AbrAlgoStrategy(func)
{
    Reset();
}

AbrBufferAlgoStrategy::~AbrBufferAlgoStrategy()
{
}

void AbrBufferAlgoStrategy::SetCurrentBitrate(int bitrate)
{
    AF_LOGI("BA already change to bitrate:%d", bitrate);
    AbrAlgoStrategy::SetCurrentBitrate(bitrate);
    mSwitching = false;
    mLastSwitchTimeMS = af_getsteady_ms();
}

void AbrBufferAlgoStrategy::Reset()
{
    mSwitching = false;
    mLastDownloadBytes = 0;
    mLastSwitchTimeMS = INT64_MIN;
    mLastBufferDuration = INT64_MIN;
    mBufferStatics.clear();
    mDownloadSpeed.clear();
    mIsUpHistory.clear();
}

void AbrBufferAlgoStrategy::ComputeBufferTrend(int64_t curTime)
{
    if (mSwitching || (1 > mBitrates.size())) {
        return;
    }

    if (mLastSwitchTimeMS == INT64_MIN) {
        return;
    }

    if (INT64_MIN == mLastBufferDuration) {
        mLastBufferDuration = mRefererData->GetCurrentPacketBufferLength() / 1000;
        return;
    }

    int64_t maxBufferDuration = mRefererData->GetMaxBufferDurationInConfig() / 1000;
    int64_t bufferDuration = mRefererData->GetCurrentPacketBufferLength() / 1000;
    bool bufferFull = (bufferDuration >= (maxBufferDuration - 1000));

    if (0 == mDurationMS) {
        bool connect = mRefererData->GetIsConnected();

//        AF_LOGI("BA connect %d full:%d", connect, bufferFull);
        if (connect) {
            int64_t remainSeg = mRefererData->GetRemainSegmentCount();

            if (0 == remainSeg) {
                bufferFull = true;
            }
        }
    }

    if ((bufferDuration > mLastBufferDuration) || bufferFull) {
        mBufferStatics.push_back(1);
    } else {
        mBufferStatics.push_back(-1);
    }

    mLastBufferDuration = bufferDuration;

    if (mBufferStatics.size() > MAX_BUFFER_STATICS_SIZE) {
        mBufferStatics.pop_front();
    }

    int bufferUp = 0;

    for (auto iter : mBufferStatics) {
        bufferUp += iter;
    }

    int64_t maxSpeed = 0;
    int64_t averageSpeed = 0;

    if (mDownloadSpeed.size() > 0) {
        std::list<int64_t> downloadSpeed = mDownloadSpeed;
        downloadSpeed.sort(std::greater<int64_t>());
        int count = 0;

        for (auto iter : downloadSpeed) {
            averageSpeed += iter;

            // ignore lowest data
            if (++count > (mDownloadSpeed.size() * 2 / 3)) {
                break;
            }
        }

        if (count) {
            averageSpeed /= count;
        }

        maxSpeed = downloadSpeed.front();
    }

    AF_LOGD("BA bufferUp:%d,bufferDuration:%lld,isFull:%d Max:%lld average:%lld", bufferUp, bufferDuration, bufferFull, maxSpeed,
            averageSpeed);

    if (((bufferDuration < LOWER_SWITCH_VALUE_MS) && (bufferUp <= -(MAX_BUFFER_STATICS_SIZE - 2)))
            || bufferDuration < 100) {
        SwitchBitrate(false, averageSpeed, maxSpeed);
    } else if ((bufferDuration >= UPPER_SWITCH_VALUE_MS || bufferFull)
               && (bufferUp >= MAX_BUFFER_STATICS_SIZE - 2)) {
        // if last BA down
        if (mIsUpHistory.size() > 0 && !mIsUpHistory.back()) {
            // wait more time
            int64_t time = af_getsteady_ms();

            if ((time - mLastSwitchTimeMS) < mUpSpan) {
                return;
            }

            // It needs to be more rigorous to BA up
            if (bufferUp < MAX_BUFFER_STATICS_SIZE) {
                return;
            }
        }

        SwitchBitrate(true, averageSpeed, maxSpeed);
    }
}

void AbrBufferAlgoStrategy::SwitchBitrate(bool up, int64_t speed, int64_t maxSpeed)
{
    int currentIndex = -1;
    int count = (int) mBitrates.size();

    for (int i = 0; i < count; i++) {
        if (mBitrates[i] == mCurrentBitrate) {
            currentIndex = i;
            break;
        }
    }

    int bitrate = -1;

    if (up) {
        if (currentIndex >= (count - 1)) {
            return;
        }

        bitrate = mBitrates[currentIndex + 1];

        if (mIsUpHistory.size() > 0 && speed > 0) {
            // last BA down
            if (!mIsUpHistory.back() && maxSpeed < bitrate) {
                AF_LOGI("last BA down, maxSpeed:%lld, nextBitrate:%d", maxSpeed, bitrate);
                return;
            }
        }

        for (int i = currentIndex + 2; i < count; ++i) {
            if (speed >= mBitrates[i]) {
                bitrate = mBitrates[i];
            }
        }
    } else {
        if (currentIndex <= 0) {
            return;
        }

        if (0 == maxSpeed) {
            bitrate = mBitrates[currentIndex - 1];
        } else {
            for (int i = currentIndex - 1; i >= 0; --i) {
                if (maxSpeed >= mBitrates[i]) {
                    bitrate = mBitrates[i];
                    break;
                }
            }

            if (-1 == bitrate) {
                bitrate = mBitrates[0];
            }
        }
    }

    if (bitrate != -1 && mCurrentBitrate != bitrate) {
        AF_LOGI("BA switch to bitrate:%d", bitrate);
        mCurrentBitrate = bitrate;
        map<int, int>::iterator iter = mStreamIndexBitrateMap.find(mCurrentBitrate);

        if (iter != mStreamIndexBitrateMap.end()) {
            mSwitching = true;
            int index = iter->second;
            mBufferStatics.clear();
            mIsUpHistory.push_back(up);

            if (!up) {
                mUpSpan = 60 * 1000;
            } else {
                mUpSpan = (MAX_BUFFER_STATICS_SIZE - 1) * 1000;
            }

            if (mIsUpHistory.size() > 3) {
                mIsUpHistory.pop_front();
            }

            mFunc(index);
        }
    }
}

void AbrBufferAlgoStrategy::ProcessAbrAlgo()
{
    if (mRefererData == nullptr || mCurrentBitrate == -1) {
        return;
    }

    int64_t curTime = af_getsteady_ms();
#ifndef WIN32

    if (mLastDownloadBytes == 0) {
        mLastDownloadBytes = mRefererData->GetDownloadedBytes();
        return;
    }

    int64_t downloadBytes = mRefererData->GetDownloadedBytes() - mLastDownloadBytes;
    int64_t downloadSpeed = downloadBytes * 8;
    mLastDownloadBytes = downloadBytes;
    mDownloadSpeed.push_back(downloadSpeed);

    if (mDownloadSpeed.size() > 30) {
        mDownloadSpeed.pop_front();
    }

#endif
    ComputeBufferTrend(curTime);
}
