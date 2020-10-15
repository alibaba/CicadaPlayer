//
// Created by moqi on 2020/1/10.
//

#ifndef CICADAMEDIA_CHEATERAUDIORENDER_H
#define CICADAMEDIA_CHEATERAUDIORENDER_H

#include "filterAudioRender.h"
#include "utils/af_clock.h"

namespace Cicada {
    class CheaterAudioRender : public filterAudioRender {
    public:
        CheaterAudioRender()
        {

        }

        ~CheaterAudioRender() override
        {

        }

    private:
        int init_device() override;

        int pause_device() override;

        int start_device() override;

        void flush_device() override;

        void device_setVolume(float gain) override
        {

        }

        int64_t device_get_position() override;

        int device_write(unique_ptr<IAFFrame> &frame) override;

        uint64_t device_get_que_duration() override;

        uint64_t device_get_ability() override
        {
            return 0;
        }

    private:
        af_clock mClock{};
        int64_t mPCMDuration{0};


    };
}


#endif //CICADAMEDIA_CHEATERAUDIORENDER_H
