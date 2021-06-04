#include "media_packet_queue.h"
#include "buffer_controller.h"
#include <cassert>
#include <cstdlib>
#include <utils/frame_work_log.h>

using namespace Cicada;
#define ADD_LOCK std::unique_lock<std::recursive_mutex> uMutex(mMutex)

MediaPacketQueue::MediaPacketQueue()
{
    mCurrent = mQueue.end();
}

MediaPacketQueue::~MediaPacketQueue()
{
    ClearQueue();
    delete mDropedExtra_data;
}

void MediaPacketQueue::ClearQueue()
{
    ADD_LOCK;
    mQueue.clear();
    mDuration = 0;
    mTotalDuration = 0;
    mPacketDuration = 0;
    mCurrent = mQueue.end();
}

void MediaPacketQueue::AddPacket(mediaPacket frame)
{
    ADD_LOCK;

    bool empty = mQueue.empty();

    if (frame->getInfo().duration > 0) {
        if (mPacketDuration == 0) {
            mPacketDuration = frame->getInfo().duration;
        }
        if (!frame->getDiscard()) {
            mDuration += frame->getInfo().duration;
            mTotalDuration += frame->getInfo().duration;
        }
    } else if (mPacketDuration > 0) {
        frame->getInfo().duration = mPacketDuration;
        if (!frame->getDiscard()) {
            mDuration += mPacketDuration;
            mTotalDuration += mPacketDuration;
        }
    }

    if (mMediaType == BUFFER_TYPE_AUDIO && !mQueue.empty() && frame->getInfo().pts != INT64_MIN &&
        frame->getInfo().pts < mQueue.back()->getInfo().pts) {
        AF_LOGE("pts revert\n");
        mQueue.back()->getInfo().dump();
        frame->getInfo().dump();
    }

    if (mDropedExtra_data && mDropedExtra_data_size > 0) {
        if (frame->getInfo().extra_data_size > 0) {
            delete mDropedExtra_data;
        } else {
            frame->getInfo().extra_data = mDropedExtra_data;
            frame->getInfo().extra_data_size = mDropedExtra_data_size;
        }
        mDropedExtra_data = nullptr;
        mDropedExtra_data_size = 0;
    }

    mQueue.push_back(move(frame));
    if (empty) {
        mCurrent = mQueue.begin();
    }
    if (mCurrent == mQueue.end()) {
        mCurrent--;
    }
}

void MediaPacketQueue::SetOnePacketDuration(int64_t duration)
{
    ADD_LOCK;
    if (mPacketDuration <= 0) {
        mPacketDuration = duration;

        int64_t missedDuration = 0;
        for (auto item = mCurrent; item != mQueue.end(); ++item) {
            if ((*item)->getInfo().duration <= 0) {
                (*item)->getInfo().duration = mPacketDuration;
                if (!(*item)->getDiscard()) {
                    missedDuration += mPacketDuration;
                }
            }
        }
        mDuration += missedDuration;

        for (auto item = mQueue.begin(); item != mCurrent; ++item) {
            if ((*item)->getInfo().duration <= 0) {
                (*item)->getInfo().duration = mPacketDuration;
                if (!(*item)->getDiscard()) {
                    missedDuration += mPacketDuration;
                }
            }
        }
        mTotalDuration += missedDuration;
    }
}

int64_t MediaPacketQueue::GetOnePacketDuration()
{
    ADD_LOCK;
    return mPacketDuration;
}

int64_t MediaPacketQueue::GetLastKeyTimePos()
{
    ADD_LOCK;
    int64_t lastPos = INT64_MIN;
    //    std::deque<std::shared_ptr<IAFPacket>>::reverse_iterator r_iter;

    for (auto r_iter = mQueue.rbegin(); r_iter != mQueue.rend(); ++r_iter) {
        IAFPacket *packet = (*r_iter).get();
        if (packet == (*mCurrent).get()) {
            break;
        }

        if (packet && (packet->getInfo().flags & AF_PKT_FLAG_KEY)) {
            lastPos = packet->getInfo().timePosition;
            return lastPos;
        }
    }

    return lastPos;
}

int64_t MediaPacketQueue::GetLastPTS()
{
    ADD_LOCK;

    if (mQueue.empty()) {
        return INT64_MIN;
    }

    if (mCurrent == mQueue.end()) {
        return INT64_MIN;
    }

    return mQueue.back()->getInfo().pts;
}

int64_t MediaPacketQueue::GetLastTimePos()
{
    ADD_LOCK;

    if (mQueue.empty()) {
        return INT64_MIN;
    }
    if (mCurrent == mQueue.end()) {
        return INT64_MIN;
    }

    return mQueue.back()->getInfo().timePosition;
}

int64_t MediaPacketQueue::GetFirstTimePos()
{
    ADD_LOCK;

    if (mQueue.empty()) {
        return INT64_MIN;
    }
    if (mCurrent == mQueue.end()) {
        return INT64_MIN;
    }
    return (*mCurrent)->getInfo().timePosition;
}

int64_t MediaPacketQueue::GetKeyPTSBefore(int64_t pts)
{
    ADD_LOCK;
    int64_t lastKeyPts = INT64_MIN;

    for (auto r_iter = mQueue.rbegin(); r_iter != mQueue.rend(); ++r_iter) {
        IAFPacket *packet = (*r_iter).get();

        if (packet && (packet->getInfo().flags & AF_PKT_FLAG_KEY) && packet->getInfo().pts <= pts) {
            lastKeyPts = packet->getInfo().pts;
            return lastKeyPts;
        }
        if (packet == (*mCurrent).get()) {
            break;
        }
    }

    return lastKeyPts;
}

int64_t MediaPacketQueue::GetKeyTimePositionBefore(int64_t pts)
{
    ADD_LOCK;
    int64_t lastKeyPos = INT64_MIN;

    for (auto r_iter = mQueue.rbegin(); r_iter != mQueue.rend(); ++r_iter) {
        IAFPacket *packet = (*r_iter).get();
        if (packet && (packet->getInfo().flags & AF_PKT_FLAG_KEY) && packet->getInfo().timePosition <= pts) {
            lastKeyPos = packet->getInfo().timePosition;
            return lastKeyPos;
        }
        if (packet == (*mCurrent).get()) {
            break;
        }
    }

    return lastKeyPos;
}

std::unique_ptr<IAFPacket> MediaPacketQueue::getPacket()
{
    ADD_LOCK;

    assert(mMAXBackwardDuration != 0 || (mTotalDuration == mDuration && mCurrent == mQueue.begin()));

    if (mQueue.empty()) {
        return nullptr;
    }

    if (mCurrent == mQueue.end()) {
        return nullptr;
    }

    std::unique_ptr<IAFPacket> packet;
    if (mMAXBackwardDuration == 0) {
        packet = move(mQueue.front());
        mQueue.pop_front();
        mCurrent = mQueue.begin();
        if (packet && packet->getInfo().duration > 0 && !packet->getDiscard()) {
            mTotalDuration -= packet->getInfo().duration;
        }
    } else {
        packet = (*mCurrent)->clone();
        mCurrent++;
    }

    if (packet && packet->getInfo().duration > 0 && !packet->getDiscard()) {
        mDuration -= packet->getInfo().duration;
    }

    if (mMAXBackwardDuration > 0) {
        while (mTotalDuration - mDuration > mMAXBackwardDuration) {
            assert(!mQueue.empty());
            bool begin = mCurrent == mQueue.begin();
            if (mQueue.front()->getInfo().duration > 0 && !mQueue.front()->getDiscard()) {
                mTotalDuration -= mQueue.front()->getInfo().duration;
            }
            mQueue.pop_front();
            if (begin) {
                mCurrent = mQueue.begin();
            }
        }

        //       AF_LOGD("mMAXBackwardDuration is %lld (ms)\n", (mTotalDuration - mDuration) / 1000);
    }

    return packet;
};

void MediaPacketQueue::PopFrontPacket()
{
    ADD_LOCK;

    if (mQueue.empty()) {
        return;
    }
    if (mCurrent == mQueue.end()) {
        return;
    }

    if ((*mCurrent) && (*mCurrent)->getInfo().duration > 0 && !(*mCurrent)->getDiscard()) {
        mDuration -= (*mCurrent)->getInfo().duration;
    }
    if ((*mCurrent)->getInfo().extra_data_size > 0) {
        AF_LOGI("save the extra_data when PopFrontPacket\n");
        delete mDropedExtra_data;
        mDropedExtra_data = (*mCurrent)->getInfo().extra_data;
        mDropedExtra_data_size = (*mCurrent)->getInfo().extra_data_size;
        (*mCurrent)->getInfo().extra_data = nullptr;
        (*mCurrent)->getInfo().extra_data_size = 0;
    }
    if (mMAXBackwardDuration == 0) {
        if (!mQueue.front()->getDiscard()) {
            mTotalDuration -= mQueue.front()->getInfo().duration;
        }
        mQueue.pop_front();
        mCurrent = mQueue.begin();
    } else {
        mCurrent++;
    }

    if (mDropedExtra_data && mDropedExtra_data_size > 0 && mCurrent != mQueue.end()) {
        if ((*mCurrent)->getInfo().extra_data_size > 0) {
            delete mDropedExtra_data;
        } else {
            (*mCurrent)->getInfo().extra_data = mDropedExtra_data;
            (*mCurrent)->getInfo().extra_data_size = mDropedExtra_data_size;
        }
        mDropedExtra_data = nullptr;
        mDropedExtra_data_size = 0;
    }
    assert(mMAXBackwardDuration != 0 || (mTotalDuration == mDuration && mCurrent == mQueue.begin()));
}

int MediaPacketQueue::GetSize()
{
    ADD_LOCK;
    // TODO:
    int size = 0;
    for (auto item = mCurrent; item != mQueue.end(); ++item) {
        size++;
    }
    return size;
}

int64_t MediaPacketQueue::FindSeamlessPointTimePosition(int &count)
{
    ADD_LOCK;
    count = 0;

    for (auto packet = mCurrent; packet != mQueue.end(); ++packet) {
        if ((*packet)->getInfo().seamlessPoint && (*packet)->getInfo().timePosition > 0) {
            return (*packet)->getInfo().timePosition;
        }

        count++;
    }

    return 0;
}

int64_t MediaPacketQueue::GetDuration()
{
    ADD_LOCK;

    if ((mMediaType == BUFFER_TYPE_VIDEO || mMediaType == BUFFER_TYPE_AUDIO) && mPacketDuration == 0) {
        if (mQueue.empty()) {
            return 0;
        }
        if (mCurrent == mQueue.end()) {
            return 0;
        }


        return -1;
    }

    return mDuration;
}

int64_t MediaPacketQueue::ClearPacketBeforePTS(int64_t pts)
{
    ADD_LOCK;
    int dropCount = 0;

    while (mCurrent != mQueue.end()) {
        IAFPacket *packet = (*mCurrent).get();

        if (packet && packet->getInfo().pts < pts) {
            PopFrontPacket();
            dropCount++;
        } else {
            break;
        }
    }

    return dropCount;
}

int64_t MediaPacketQueue::ClearPacketBeforeTimePos(int64_t pts)
{
    ADD_LOCK;
    int dropCount = 0;
    while (mCurrent != mQueue.end()) {
        IAFPacket *packet = (*mCurrent).get();

        if (packet && packet->getInfo().timePosition < pts) {
            PopFrontPacket();
            dropCount++;
        } else {
            break;
        }
    }

    return dropCount;
}

void MediaPacketQueue::ClearPacketAfterTimePosition(int64_t pts)
{
    ADD_LOCK;
    bool found = false;

    while (mQueue.back() != (*mCurrent) && !found) {
        IAFPacket *packet = mQueue.back().get();

        if (packet == nullptr) {
            mQueue.pop_back();
            continue;
        }

        if (packet->getInfo().timePosition == pts) {
            found = true;
        }

        if (packet->getInfo().duration > 0 && !packet->getDiscard()) {
            mDuration -= packet->getInfo().duration;
            mTotalDuration -= packet->getInfo().duration;
        }

        mQueue.pop_back();
    }

    if (!found) {
        AF_LOGE("pts not found");
    } else {
        AF_LOGE("pts %lld found", pts);
    }

    if (!mQueue.empty() && mCurrent != mQueue.end()) {
        if (mMediaType == BUFFER_TYPE_AUDIO) {
            AF_LOGD("audio change last pts is %lld\n", mQueue.back()->getInfo().pts);
        } else {
            AF_LOGD("video change last pts is %lld\n", mQueue.back()->getInfo().pts);
        }
    }
}

int64_t MediaPacketQueue::GetPts()
{
    ADD_LOCK;

    if (mQueue.empty()) {
        return INT64_MIN;
    }
    if (mCurrent == mQueue.end()) {
        return INT64_MIN;
    }

    return mQueue.front()->getInfo().pts;
}
void MediaPacketQueue::Rewind()
{
    mCurrent = mQueue.begin();
    mDuration = mTotalDuration;
}
