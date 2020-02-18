#include <cstdlib>
#include <utils/frame_work_log.h>
#include "media_packet_queue.h"
#include "buffer_controller.h"

namespace Cicada {
#define ADD_LOCK std::unique_lock<std::recursive_mutex> uMutex(mMutex)

    MediaPacketQueue::MediaPacketQueue() = default;

    MediaPacketQueue::~MediaPacketQueue()
    {
        ClearQueue();
    }

    void MediaPacketQueue::ClearQueue()
    {
        ADD_LOCK;
        mQueue.clear();
        mDuration = 0;
        mPacketDuration = 0;
    }

    void MediaPacketQueue::AddPacket(mediaPacket frame)
    {
        ADD_LOCK;

        if (frame->getInfo().duration > 0) {
            if (mPacketDuration == 0) {
                mPacketDuration = frame->getInfo().duration;
            }

            mDuration += frame->getInfo().duration;
        }

        if (mMediaType == BUFFER_TYPE_AUDIO && !mQueue.empty() && frame->getInfo().pts != INT64_MIN
                && frame->getInfo().pts < mQueue.back()->getInfo().pts) {
            AF_LOGE("pts revert\n");
            mQueue.back()->getInfo().dump();
            frame->getInfo().dump();
        }

        mQueue.push_back(move(frame));
    }


    int64_t MediaPacketQueue::GetLastKeyTimePos()
    {
        ADD_LOCK;
        int64_t lastPos = INT64_MIN;
        //    std::deque<std::shared_ptr<IAFPacket>>::reverse_iterator r_iter;

        for (auto r_iter = mQueue.rbegin(); r_iter != mQueue.rend(); ++r_iter) {
            IAFPacket *packet = (*r_iter).get();

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

        return mQueue.back()->getInfo().pts;
    }

    int64_t MediaPacketQueue::GetLastTimePos()
    {
        ADD_LOCK;

        if (mQueue.empty()) {
            return INT64_MIN;
        }

        return mQueue.back()->getInfo().timePosition;
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
        }

        return lastKeyPos;
    }

    std::unique_ptr<IAFPacket> MediaPacketQueue::getPacket()
    {
        ADD_LOCK;

        if (mQueue.empty()) {
            return nullptr;
        }

        std::unique_ptr<IAFPacket> packet = move(mQueue.front());
        mQueue.pop_front();

        if (packet && packet->getInfo().duration > 0) {
            mDuration -= packet->getInfo().duration;
        }

        return packet;
    };

    void MediaPacketQueue::PopFrontPacket()
    {
        ADD_LOCK;

        if (mQueue.size() == 0) {
            return;
        }

        if (mQueue.front() && mQueue.front()->getInfo().duration > 0) {
            mDuration -= mQueue.front()->getInfo().duration;
        }

        mQueue.pop_front();
    }

    int MediaPacketQueue::GetSize()
    {
        ADD_LOCK;
        return (int) mQueue.size();
    }

    int64_t MediaPacketQueue::FindSeamlessPointTimePosition(int &count)
    {
        ADD_LOCK;
        count = 0;

        for (auto &packet : mQueue) {
            if (packet->getInfo().seamlessPoint && packet->getInfo().timePosition > 0) {
                return packet->getInfo().timePosition;
            }

            count++;
        }

        return 0;
    }

    int64_t MediaPacketQueue::GetDuration()
    {
        ADD_LOCK;

        if (mMediaType == BUFFER_TYPE_VIDEO && mPacketDuration == 0) {
            if (mQueue.empty()) {
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

        while (!mQueue.empty()) {
            IAFPacket *packet = mQueue.front().get();

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

        while (!mQueue.empty()) {
            IAFPacket *packet = mQueue.front().get();

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

        while (!mQueue.empty() && !found) {
            IAFPacket *packet = mQueue.back().get();

            if (packet == nullptr) {
                mQueue.pop_back();
                continue;
            }

            if (packet->getInfo().timePosition == pts) {
                found = true;
            }

            if (packet->getInfo().duration > 0) {
                mDuration -= packet->getInfo().duration;
            }

            mQueue.pop_back();
        }

        if (!found) {
            AF_LOGE("pts not found");
        } else {
            AF_LOGE("pts %lld found", pts);
        }

        if (!mQueue.empty()) {
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

        return mQueue.front()->getInfo().pts;
    }

}//namespace Cicada
