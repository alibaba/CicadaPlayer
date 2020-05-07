//
// Created by moqi on 2018/9/21.
//

#ifndef CICADA_PLAYER_ICICADAPLAYER_H
#define CICADA_PLAYER_ICICADAPLAYER_H

#include <utils/AFMediaType.h>
#include "native_cicada_player_def.h"
#include <cacheModule/cache/CacheConfig.h>

namespace Cicada{
    class IDemuxerFactory;
}

namespace Cicada {

    class ICicadaPlayer {
    public:

        ICicadaPlayer() = default;

        virtual ~ICicadaPlayer() = default;

        /*
        * 初始化;(同步)
        */
        virtual int SetListener(const playerListener &Listener) = 0;


        virtual void SetOnRenderCallBack(onRenderFrame cb, void *userData) = 0;

        /*
         *设置显示窗口
         */
        virtual void SetView(void *view) = 0;

        /*
         *设置播放url
         */
        virtual void SetDataSource(const char *url) = 0;

        /*
        * 准备播放
        * url  播放url
        * startPos 播放起始位置
        * key 加密key
        * circleCount 加密count循环次数
        */
        virtual void Prepare() = 0;

        /*
        * 开始播放
        */
        virtual void Start() = 0;

        /*
         * 暂停播放
         */
        virtual void Pause() = 0;


        virtual StreamType SwitchStream(int index) = 0;

        /*
         * 跳转到指定位置
         * seekPos 跳转位置
         * bAccurate 是否精准seek
         */
        virtual void SeekTo(int64_t seekPos, bool bAccurate = false) = 0;

        /*
         * 停止播放
         */
        virtual int Stop() = 0;

        /*
         * 获取播放状态
         */
        virtual PlayerStatus GetPlayerStatus() const = 0;

        /*
         * 获取视频长度
         */
        virtual int64_t GetDuration() const = 0;

        /*
         * 获取视频当前播放位置
         */
        virtual int64_t GetPlayingPosition() = 0;

        /*
         * 获取视频当前缓存位置
         */
        virtual int64_t GetBufferPosition() = 0;

        /*
         * 设置静音
         */
        virtual void Mute(bool bMute) = 0;

        virtual bool IsMute() const = 0;

        /*
         * 设置音量
         */
        virtual void SetVolume(float volume) = 0;

        virtual float GetVideoRenderFps() = 0;

        virtual void EnterBackGround(bool back) = 0;

        virtual void SetScaleMode(ScaleMode mode) = 0;

        virtual ScaleMode GetScaleMode() = 0;

        virtual void SetRotateMode(RotateMode mode) = 0;

        virtual RotateMode GetRotateMode() = 0;

        virtual void SetMirrorMode(MirrorMode mode) = 0;

        virtual void SetVideoBackgroundColor(uint32_t color) = 0;

        virtual MirrorMode GetMirrorMode() = 0;

        virtual int GetCurrentStreamIndex(StreamType type) = 0;

        virtual StreamInfo *GetCurrentStreamInfo(StreamType type) = 0;

        virtual int64_t GetMasterClockPts() = 0;

        /*
         * 设置网络超时时间
         */
        virtual void SetTimeout(int timeout) = 0;

        /*
         * 设置丢帧时间
         */
        virtual void SetDropBufferThreshold(int dropValue) = 0;


        virtual void SetDecoderType(DecoderType type) = 0;

        virtual DecoderType GetDecoderType() = 0;

        virtual float GetVolume() const = 0;

        /*
         * 设置referer
         */
        virtual void SetRefer(const char *refer) = 0;

        /*
         * 设置user Agent
         */
        virtual void SetUserAgent(const char *userAgent) = 0;

        /*
         * 设置循环播放
         */
        virtual void SetLooping(bool bCirclePlay) = 0;

        /*
         * 获取是否循环播放
         */
        virtual bool isLooping() = 0;

        /*
         * 截屏
         */
        virtual void CaptureScreen() = 0;

        /*
         * 获取视频分辨率
         */
        virtual void GetVideoResolution(int &width, int &height) = 0;

        /*
         * 获取视频旋转角度
         */
        virtual void GetVideoRotation(int &rotation) = 0;

        virtual std::string GetPropertyString(PropertyKey key)
        {
            return "";
        };

        virtual int64_t GetPropertyInt(PropertyKey key)
        {
            return 0;
        };

        virtual long GetPropertyLong(int key)
        {
            return 0;
        };

        virtual float GetVideoDecodeFps() = 0;

        virtual int SetOption(const char *key, const char *value) = 0;

        virtual void GetOption(const char *key, char *value) = 0;

        virtual void setSpeed(float speed) = 0;

        virtual float getSpeed() = 0;

        virtual void AddCustomHttpHeader(const char *httpHeader) = 0;

        virtual void RemoveAllCustomHttpHeader() = 0;

        virtual void addExtSubtitle(const char *uri) = 0;

        virtual int selectExtSubtitle(int index, bool bSelect) = 0;

        virtual void setMediaFrameCb(playerMediaFrameCb func, void *arg)
        {
            if (mMediaFrameCb == func) {
                return;
            }
            mMediaFrameCbArg = arg;
            mMediaFrameCb = func;
        }

        virtual void setBitStreamCb(readCB read, seekCB seek, void *arg)
        {
            mBSReadCb = read;
            mBSSeekCb = seek;
            mBSCbArg = arg;
        }

        virtual void setClockRefer(clockRefer cb, void *arg)
        {
            mClockRef = cb;
            mCRArg = arg;
        }

        virtual int getCurrentStreamMeta(Stream_meta *meta, StreamType type) = 0;

        virtual void setErrorConverter(ErrorConverter *converter)
        {
            mErrorConverter = converter;
        }

        /*
         * Reload network connection at the break point
         */

        virtual void reLoad() = 0;

        virtual void SetAutoPlay(bool bAutoPlay) = 0;

        virtual bool IsAutoPlay() = 0;

    protected:
        playerMediaFrameCb mMediaFrameCb = nullptr;
        void *mMediaFrameCbArg = nullptr;

        readCB mBSReadCb = nullptr;
        seekCB mBSSeekCb = nullptr;
        void *mBSCbArg = nullptr;
        clockRefer mClockRef = nullptr;
        void* mCRArg = nullptr;

        ErrorConverter *mErrorConverter = nullptr;
    };

}


#endif //CICADA_PLAYER_ICICADAPLAYER_H
