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
        {}

        ~CheaterAudioRender() override
        {}

    private:
        int init_device() override;

        int pause_device() override;

        int start_device() override;

        void flush_device() override;

        void device_setVolume(float gain) override
        {}

        void device_mute(bool bMute) override
        {}

        int64_t device_get_position() override;

        int device_write(unique_ptr<IAFFrame> &frame) override;

        uint64_t device_get_que_duration() override;

        uint64_t device_get_ability() override
        {
            return 0;
        }
        bool device_require_format(const IAFFrame::audioInfo &info) override
        {
            mInputInfo = info;
            return true;
        }
        void device_mute(bool bMute)
        {
            return;
        }

    private:
        class positionInfo {
        public:
            positionInfo(unique_ptr<IAFFrame> &frame, int64_t position) : mFrame(std::move(frame)), mPosition(position)
            {}
            ~positionInfo() = default;

            int64_t getPosition() const
            {
                return mPosition;
            }

            IAFFrame *getFrame()
            {
                return mFrame.get();
            }

        private:
            unique_ptr<IAFFrame> mFrame;
            int64_t mPosition;
        };

    private:
        af_clock mClock{};
        int64_t mPCMDuration{0};
        std::mutex mMutex;
        std::queue<std::unique_ptr<positionInfo>> mQueue;
    };
}// namespace Cicada


#endif//CICADAMEDIA_CHEATERAUDIORENDER_H
