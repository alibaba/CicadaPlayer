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
            return 0;
        }
        int setRotate(Rotate rotate) override
        {
            return 0;
        }
        int setFlip(Flip flip) override
        {
            return 0;
        }
        int setScale(Scale scale) override
        {
            return 0;
        }

        uint64_t getFlags() override
        {
            return FLAG_DUMMY;
        }
    };
}// namespace Cicada


#endif//CICADAMEDIA_DUMMYVIDEORENDER_H
