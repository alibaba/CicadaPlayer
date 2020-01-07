//
//  AutoAVFrame.h
//  Support auto release the AVFrame
//
//  Created by huang_jiafa on 2018/12/19.
//  Copyright © 2018 Aliyun. All rights reserved.
//

#ifndef AutoAVFrame_H
#define AutoAVFrame_H

extern "C" {
#include <libavutil/frame.h>
};

class AutoAVFrame {
public:
    AutoAVFrame(AVFrame *frame):mFrame(frame) {}

    ~AutoAVFrame() {
        if (mRelease && mFrame) {
            av_frame_unref(mFrame);
            av_frame_free(&mFrame);
        }
    }

    void setAutoRelease(bool release) {
        mRelease = release;
    }

    AVFrame *getFrame() {return mFrame;}

private:

    AutoAVFrame(const AutoAVFrame &);
    const AutoAVFrame &operator=(const AutoAVFrame &);

    bool mRelease = true;
    AVFrame *mFrame = nullptr;
};


#endif //AutoAVFrame_H
