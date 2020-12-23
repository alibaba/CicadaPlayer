//
// Created by pingkai on 2020/11/2.
//

#ifndef CICADAMEDIA_SMPAVDEVICEMANAGER_H
#define CICADAMEDIA_SMPAVDEVICEMANAGER_H

#include <codec/IDecoder.h>
#include <render/audio/IAudioRender.h>
#include <render/video/IVideoRender.h>

#ifdef __APPLE__
#include <codec/Apple/AppleVideoToolBox.h>
#endif


// TODO: add create lock
namespace Cicada {
    class SMPAVDeviceManager {
    public:
        enum deviceType { DEVICE_TYPE_VIDEO = 1 << 0, DEVICE_TYPE_AUDIO = 1 << 1 };

        struct DecoderHandle {
            std::unique_ptr<IDecoder> decoder{nullptr};
            Stream_meta meta{};
            bool valid{false};
            uint64_t decFlag;
            void *device;
            uint32_t mDstFormat{0};
            std::unique_ptr<DrmInfo> mDrmInfo{nullptr};

            bool match(const Stream_meta *pMeta, uint64_t flag, void *pDevice, uint32_t dstFormat,
                       const DrmInfo *info)
            {
#ifdef __APPLE__
                auto *vtbDecoder = dynamic_cast<AFVTBDecoder *>(decoder.get());
                if (vtbDecoder) {
                    if (pMeta->interlaced) {
                        return false;
                    }
                }
#endif
                return (pDevice == device) && (flag == decFlag) && (pMeta->codec == meta.codec) && (dstFormat == mDstFormat)
                 && ((mDrmInfo == nullptr && info == nullptr ) || (info!= nullptr && mDrmInfo.get()!= nullptr && *info == *mDrmInfo.get()));
            }
        };

    public:
        SMPAVDeviceManager();
        ~SMPAVDeviceManager();

        IDecoder *getDecoder(deviceType type) const
        {
            if (type == DEVICE_TYPE_VIDEO) {
                return mVideoDecoder.decoder.get();
            } else if (type == DEVICE_TYPE_AUDIO) {
                return mAudioDecoder.decoder.get();
            }
            return nullptr;
        }

        bool isDecoderValid(deviceType type) const
        {
            if (type == DEVICE_TYPE_VIDEO) {
                return mVideoDecoder.valid;
            } else if (type == DEVICE_TYPE_AUDIO) {
                return mAudioDecoder.valid;
            }
            return false;
        }
        bool isAudioRenderValid() const
        {
            return mAudioRenderValid;
        }

        void invalidDevices(uint64_t deviceTypes);

        void flushDevice(uint64_t deviceTypes);

        int getFrame(std::unique_ptr<IAFFrame> &frame, deviceType type, uint64_t timeOut);

        int sendPacket(std::unique_ptr<IAFPacket> &packet, deviceType type, uint64_t timeOut);

        int setVolume(float volume);

        void setMute(bool mute);

        uint64_t getAudioRenderQueDuration();

        int renderAudioFrame(std::unique_ptr<IAFFrame> &frame, int timeOut);

        void pauseAudioRender(bool pause);

        int setUpAudioRender(const IAFFrame::audioInfo &info);

        int setSpeed(float speed);

        int64_t getAudioRenderPosition();

        void setAudioRenderListener(IAudioRenderListener *listener);

        void setVideoRenderListener(IVideoRender::IVideoRenderListener *listener);

        void setAudioRenderingCb(renderingFrameCB cb, void *userData);

        int setUpDecoder(uint64_t decFlag, const Stream_meta *meta, void *device, deviceType type, uint32_t dstFormat);

        uint64_t getVideoDecoderFlags();

        int createVideoRender(uint64_t flags);

        void destroyVideoRender();

        bool isVideoRenderValid()
        {
            return mVideoRenderValid;
        }
        IVideoRender *getVideoRender()
        {
            if (mVideoRender) {
                return mVideoRender.get();
            }
            return nullptr;
        }
        void flushVideoRender();

        int renderVideoFrame(std::unique_ptr<IAFFrame> &frame);

        void setRequireDrmHandlerCallback(const std::function<DrmHandler*(const DrmInfo& drmInfo)>& callback);

    private:
        DecoderHandle *getDecoderHandle(const deviceType &type);

    private:
        std::mutex mMutex{};
        DecoderHandle mAudioDecoder;
        DecoderHandle mVideoDecoder;
        std::unique_ptr<IAudioRender> mAudioRender{nullptr};
        IAFFrame::audioInfo mAudioRenderInfo{};
        bool mAudioRenderValid{false};
        bool mMute{false};
        std::unique_ptr<IVideoRender> mVideoRender{nullptr};
        bool mVideoRenderValid{false};
        uint64_t mVideoRenderFlags{0};
        std::function<DrmHandler*(const DrmInfo& drmInfo)> mRequireDrmHandlerCallback{nullptr};
    };
}// namespace Cicada


#endif//CICADAMEDIA_SMPAVDEVICEMANAGER_H
