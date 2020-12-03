#define LOG_TAG "BufferControlService"

#include <utils/frame_work_log.h>
#include <cassert>
#include "buffer_controller.h"

using namespace std;
namespace Cicada {
    BufferController::BufferController()
    {
        mAudioPacketQueue.mMediaType = BUFFER_TYPE_AUDIO;
        mVideoPacketQueue.mMediaType = BUFFER_TYPE_VIDEO;
    }

    BufferController::~BufferController()
    {
        mVideoPacketQueue.ClearQueue();
        mAudioPacketQueue.ClearQueue();
        mSubtitlePacketQueue.ClearQueue();
    }

    void BufferController::AddPacket(unique_ptr<IAFPacket> packet, BUFFER_TYPE type)
    {
        if (packet == nullptr) {
            return;
        }

        switch (type) {
            case BUFFER_TYPE_AUDIO:
                return mAudioPacketQueue.AddPacket(move(packet));

            case BUFFER_TYPE_VIDEO:
                return mVideoPacketQueue.AddPacket(move(packet));

            case BUFFER_TYPE_SUBTITLE:
                return mSubtitlePacketQueue.AddPacket(move(packet));

            default:
                AF_LOGE("error media type");
                break;
        }
    }

    void BufferController::SetOnePacketDuration(BUFFER_TYPE type, int64_t duration) {
        switch (type) {
            case BUFFER_TYPE_AUDIO:
                mAudioPacketQueue.SetOnePacketDuration(duration);
                break;
            case BUFFER_TYPE_VIDEO:
                mVideoPacketQueue.SetOnePacketDuration(duration);
                break;
            case BUFFER_TYPE_SUBTITLE:
                break;
            default:
                AF_LOGE("error media type");
                break;
        }
    }


    int64_t BufferController::GetOnePacketDuration(BUFFER_TYPE type) {
        switch (type) {
            case BUFFER_TYPE_AUDIO:
                return mAudioPacketQueue.GetOnePacketDuration();
            case BUFFER_TYPE_VIDEO:
                return mVideoPacketQueue.GetOnePacketDuration();
            case BUFFER_TYPE_SUBTITLE:
                break;
            default:
                AF_LOGE("error media type");
                break;
        }
        return INT64_MIN;
    }


    int64_t BufferController::GetPacketPts(BUFFER_TYPE type)
    {
        switch (type) {
            case BUFFER_TYPE_AUDIO:
                return mAudioPacketQueue.GetPts();

            case BUFFER_TYPE_VIDEO:
                return mVideoPacketQueue.GetPts();

            case BUFFER_TYPE_SUBTITLE:
                return mSubtitlePacketQueue.GetPts();

            default:
                AF_LOGE("error media type");
                break;
        }

        return INT64_MIN;
    }

    int64_t BufferController::GetKeyPTSBefore(BUFFER_TYPE type, int64_t pts)
    {
        switch (type) {
            case BUFFER_TYPE_AUDIO:
                return mAudioPacketQueue.GetKeyPTSBefore(pts);

            case BUFFER_TYPE_VIDEO:
                return mVideoPacketQueue.GetKeyPTSBefore(pts);

            default:
                AF_LOGE("error media type");
                break;
        }

        return INT64_MIN;
    }

    int64_t BufferController::GetKeyTimePositionBefore(BUFFER_TYPE type, int64_t pts)
    {
        switch (type) {
            case BUFFER_TYPE_AUDIO:
                return mAudioPacketQueue.GetKeyTimePositionBefore(pts);

            case BUFFER_TYPE_VIDEO:
                return mVideoPacketQueue.GetKeyTimePositionBefore(pts);

            default:
                AF_LOGE("error media type");
                break;
        }

        return INT64_MIN;
    }

    int64_t BufferController::ClearPacketBeforePts(BUFFER_TYPE type, int64_t pts)
    {
        int64_t drop = 0;

        if (type & BUFFER_TYPE_AUDIO) {
            drop += mAudioPacketQueue.ClearPacketBeforePTS(pts);
        }

        if (type & BUFFER_TYPE_VIDEO) {
            drop += mVideoPacketQueue.ClearPacketBeforePTS(pts);
        }

        if (type & BUFFER_TYPE_SUBTITLE) {
            drop += mSubtitlePacketQueue.ClearPacketBeforePTS(pts);
        }

        return drop;
    }

    int64_t BufferController::ClearPacketBeforeTimePos(BUFFER_TYPE type, int64_t pts)
    {
        int64_t drop = 0;

        if (type & BUFFER_TYPE_AUDIO) {
            drop += mAudioPacketQueue.ClearPacketBeforeTimePos(pts);
        }

        if (type & BUFFER_TYPE_VIDEO) {
            drop += mVideoPacketQueue.ClearPacketBeforeTimePos(pts);
        }

        if (type & BUFFER_TYPE_SUBTITLE) {
            drop += mSubtitlePacketQueue.ClearPacketBeforeTimePos(pts);
        }

        return drop;
    }

    void BufferController::ClearPacketAfterTimePosition(BUFFER_TYPE type, int64_t pts)
    {
        if (type & BUFFER_TYPE_AUDIO) {
            mAudioPacketQueue.ClearPacketAfterTimePosition(pts);
        }

        if (type & BUFFER_TYPE_VIDEO) {
            mVideoPacketQueue.ClearPacketAfterTimePosition(pts);
        }

        if (type & BUFFER_TYPE_SUBTITLE) {
            mSubtitlePacketQueue.ClearPacketAfterTimePosition(pts);
        }
    }

    int64_t BufferController::GetPacketLastKeyTimePos(BUFFER_TYPE type)
    {
        switch (type) {
            case BUFFER_TYPE_AUDIO:
                return mAudioPacketQueue.GetLastKeyTimePos();

            case BUFFER_TYPE_VIDEO:
                return mVideoPacketQueue.GetLastKeyTimePos();

            case BUFFER_TYPE_SUBTITLE:
                return mSubtitlePacketQueue.GetLastKeyTimePos();

            default:
                AF_LOGE("error media type");
                break;
        }

        return 0;
    }

    int64_t BufferController::GetPacketLastPTS(BUFFER_TYPE type)
    {
        switch (type) {
            case BUFFER_TYPE_AUDIO:
                return mAudioPacketQueue.GetLastPTS();

            case BUFFER_TYPE_VIDEO:
                return mVideoPacketQueue.GetLastPTS();

            case BUFFER_TYPE_SUBTITLE:
                return mSubtitlePacketQueue.GetLastPTS();

            default:
                AF_LOGE("error media type");
                break;
        }

        return 0;
    }

    int64_t BufferController::GetPacketLastTimePos(BUFFER_TYPE type)
    {
        switch (type) {
            case BUFFER_TYPE_AUDIO:
                return mAudioPacketQueue.GetLastTimePos();

            case BUFFER_TYPE_VIDEO:
                return mVideoPacketQueue.GetLastTimePos();

            case BUFFER_TYPE_SUBTITLE:
                return mSubtitlePacketQueue.GetLastTimePos();

            default:
                AF_LOGE("error media type");
                break;
        }

        return 0;
    }

    int64_t BufferController::FindSeamlessPointTimePosition(BUFFER_TYPE type, int &count)
    {
        switch (type) {
            case BUFFER_TYPE_AUDIO:
                return mAudioPacketQueue.FindSeamlessPointTimePosition(count);

            case BUFFER_TYPE_VIDEO:
                return mVideoPacketQueue.FindSeamlessPointTimePosition(count);

            case BUFFER_TYPE_SUBTITLE:
                return mSubtitlePacketQueue.FindSeamlessPointTimePosition(count);

            default:
                AF_LOGE("error media type");
                break;
        }

        return 0;
    }

    int64_t BufferController::GetPacketDuration(BUFFER_TYPE type)
    {
        switch (type) {
            case BUFFER_TYPE_AUDIO:
                return mAudioPacketQueue.GetDuration();

            case BUFFER_TYPE_VIDEO:
                return mVideoPacketQueue.GetDuration();

            case BUFFER_TYPE_SUBTITLE:
                return mSubtitlePacketQueue.GetDuration();

            default:
                AF_LOGE("error media type");
                break;
        }

        return 0;
    }

    bool BufferController::IsPacketEmtpy(BUFFER_TYPE type)
    {
        switch (type) {
            case BUFFER_TYPE_AUDIO:
                return mAudioPacketQueue.GetSize() == 0;

            case BUFFER_TYPE_VIDEO:
                return mVideoPacketQueue.GetSize() == 0;

            case BUFFER_TYPE_SUBTITLE:
                return mSubtitlePacketQueue.GetSize() == 0;

            default:
                AF_LOGE("error media type");
                break;
        }

        return true;
    }

    std::unique_ptr<IAFPacket> BufferController::getPacket(BUFFER_TYPE type)
    {
        switch (type) {
            case BUFFER_TYPE_AUDIO:
                return mAudioPacketQueue.getPacket();

            case BUFFER_TYPE_VIDEO:
                return mVideoPacketQueue.getPacket();

            case BUFFER_TYPE_SUBTITLE:
                return mSubtitlePacketQueue.getPacket();

            default:
                AF_LOGE("error media type");
                break;
        }

        return nullptr;
    }

    int BufferController::GetPacketSize(BUFFER_TYPE type)
    {
        int size = 0;

        if (type & BUFFER_TYPE_AUDIO) {
            size += mAudioPacketQueue.GetSize();
        }

        if (type & BUFFER_TYPE_VIDEO) {
            size += mVideoPacketQueue.GetSize();
        }

        if (type & BUFFER_TYPE_SUBTITLE) {
            size += mSubtitlePacketQueue.GetSize();
        }

        return size;
    }

    void BufferController::ClearPacket(BUFFER_TYPE type)
    {
        if (type & BUFFER_TYPE_AUDIO) {
            mAudioPacketQueue.ClearQueue();
        }

        if (type & BUFFER_TYPE_VIDEO) {
            mVideoPacketQueue.ClearQueue();
        }

        if (type & BUFFER_TYPE_SUBTITLE) {
            mSubtitlePacketQueue.ClearQueue();
        }
    }


}//namespace Cicada
