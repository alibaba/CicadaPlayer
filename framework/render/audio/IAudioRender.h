//
// Created by lifujun on 2019/7/23.
//

#ifndef FRAMEWORK_IAudioRender_H
#define FRAMEWORK_IAudioRender_H


#include <base/media/IAFPacket.h>
#include <utils/AFMediaType.h>

namespace Cicada{
    class IAFRenderFilter {
    public:
        virtual ~IAFRenderFilter() = default;

        virtual int push(std::unique_ptr<IAFFrame> &frame) = 0;

        virtual int pull(std::unique_ptr<IAFFrame> &frame) = 0;
    };

    class IAudioRenderListener {
    public:
        /**
         * playback eos
         */
        virtual void onEOS() = 0;

        /**
         * playback interrupt
         */
        virtual void onInterrupt(bool interrupt) {};
    };

    class IAudioRender {

    public:

        const static int FORMAT_NOT_SUPPORT = -201;

        virtual ~IAudioRender() = default;

        /**
         * get the render codec supported,
         * bitStream support is option, pcm is MUST.
         * user can send the bit stream directly when  bitStream was supported, and render can pass through
         * it to output.
         */

        virtual bool isSupport(AFCodecID id)
        {
            return false;
        };

        /**
         *  init the render, not start to play after init success
         */

        virtual int init(const IAFFrame::audioInfo *info) = 0;


        /**
         * add pcm to render
         * @param timeOut  time to wait when render is full.
         *                 -1, blocking
         * @param frame    use nullptr to send eos.
         *
         * @return
         *       0       success
         *       -EAGAIN render is full, try again
         */

        virtual int renderFrame(std::unique_ptr<IAFFrame> &frame, int timeOut) = 0;


        /**
         * add bit steam packet to render
         * @param timeOut, time to wait when render is full.
         *               -1, blocking
         * @param packet    use nullptr to send eos.
         *
         * @return
         *       0       success
         *       -EAGAIN render is full, try again
         */

        virtual int renderFrame(std::unique_ptr<IAFPacket> &packet, int timeOut) = 0;

        /**
         * get the played position in us.
         * not pts, will be reset to zero when flush
         *
         * @return
         *      INT64_MIN not available
         *
         */

        virtual int64_t getPosition() = 0;

        /**
         * mute or not mute the playback. NOT change the system param.
         * this is independent of the volume. not equal to set volume to zero.
         */

        virtual void mute(bool bMute) = 0;

        /**
         * set the volume of this audio playback. NOT change the system param,
         * this can not be supported when in pass-through mode
         *
         * @para volume
         *       [0 ~ 1.0) reduce the volume
         *       1.0       the origin volume, the default value.
         *       > 1.0     raise the origin volume by soft ware, this would give rise to
         *                 distortion.
         *
         * @return
         *       0      success
         *       < 0    not support
         */

        virtual int setVolume(float volume) = 0;

        /**
         * set the play back speed, this would give rise to distortion,
         * this can not be supported when in pass-through mode.
         *
         * @param speed
         *             [0~ 1.0) slow down the playback
         *             1.0      the origin speed, the default value.
         *             >1.0     speed up the playback.
         * @return
         *       0      success
         *       < 0    not support
         */

        virtual int setSpeed(float speed) = 0;

        /**
         * pause/resume the play back
         */

        virtual void pause(bool bPause) = 0;

        /**
         * drop all the data not be played back, the position will set to zero
         */

        virtual void flush() = 0;

        /**
         *
         * @return get the real duration queued but not played
         */
        virtual uint64_t getQueDuration() = 0;

        virtual void setFilter(IAudioRender *filter)
        {
            mExtFilter = filter;
        }

        virtual void setListener(IAudioRenderListener *listener)
        {
            mListener = listener;
        }

    protected:
        IAudioRender *mExtFilter{};
        IAudioRenderListener *mListener{};
    };
}

#endif //FRAMEWORK_IAudioRender_H
