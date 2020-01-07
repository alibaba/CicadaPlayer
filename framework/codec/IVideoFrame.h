
#ifndef IVideoFrame_H
#define IVideoFrame_H

#include <stdlib.h>
#include <stdint.h>

struct AVFrame;

namespace Cicada{
    typedef enum VIDEO_FRAME_TYPE {
        VIDEO_FRAME_FFMPEG,
        VIDEO_FRAME_CICADA,
    } VIDEO_FRAME_TYPE;

    class IVideoFrame {
    public:

        IVideoFrame(AVFrame *frame) {
            mType = VIDEO_FRAME_FFMPEG;
            mAVFrame = frame;
        }

        // only keep the user pointer
        IVideoFrame(void *frame, int64_t pts) {
            mType = VIDEO_FRAME_CICADA;
            mUserInfo = frame;
            mPts = pts;
        }

        ~IVideoFrame();

        VIDEO_FRAME_TYPE getType() {
            return mType;
        }

        int64_t getPts();

        int * getLinesize();

        uint8_t ** getBuffer();

        void *getUserInfo();

    private:
        VIDEO_FRAME_TYPE mType = VIDEO_FRAME_FFMPEG;
        AVFrame *mAVFrame = nullptr;

        void* mUserInfo = nullptr;
        int64_t mPts = INT64_MIN;
    };
}


#endif //IVideoFrame_H
