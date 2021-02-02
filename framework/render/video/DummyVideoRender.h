//
// Created by pingkai on 2021/2/2.
//

#ifndef CICADAMEDIA_DUMMYVIDEORENDER_H
#define CICADAMEDIA_DUMMYVIDEORENDER_H

#include "AFActiveVideoRender.h"

namespace Cicada {
    class DummyVideoRender : public AFActiveVideoRender {

    public:
        ~DummyVideoRender() override = default;

        bool deviceRenderFrame(IAFFrame *frame) override
        {
            return true;
        }

        int init() override
        {
            return 0;
        }
        int clearScreen() override
        {
            if (mVideoRenderOperationListener != nullptr) {
                mVideoRenderOperationListener->onClearScreen();
            }
            return 0;
        }

        void setBackgroundColor(uint32_t color) override
        {
            if (mVideoRenderOperationListener != nullptr) {
                mVideoRenderOperationListener->onSetBackgroundColor(color);
            }
        };
        int setRotate(Rotate rotate) override
        {
            if (mVideoRenderOperationListener != nullptr) {
                mVideoRenderOperationListener->onSetRotateMode(rotate);
            }
            return 0;
        }
        int setFlip(Flip flip) override
        {
            if (mVideoRenderOperationListener != nullptr) {
                mVideoRenderOperationListener->onSetMirrorMode(flip);
            }
            return 0;
        }
        int setScale(Scale scale) override
        {
            if (mVideoRenderOperationListener != nullptr) {
                mVideoRenderOperationListener->onSetScaleMode(scale);
            }
            return 0;
        }

        uint64_t getFlags() override
        {
            return FLAG_DUMMY;
        }
    };
}// namespace Cicada


#endif//CICADAMEDIA_DUMMYVIDEORENDER_H
