//
//  MediaPlayer.h
//  apsara_player
//
//  Created by shiping.csp on 2018/11/12.
//

#ifndef CicadaPlayer_h
#define CicadaPlayer_h

#include "MediaPlayerConfig.h"
#include "native_cicada_player_def.h"
#include <stdio.h>

class AbrManager;

class AbrAlgoStrategy;

#include <cacheModule/CacheManager.h>
#include <mutex>
#include <sstream>
#include <string>
#include <utils/AFMediaType.h>
#include <vector>

#ifdef ANDROID

namespace std {
    template<typename T>
    std::string to_string(T value)
    {
        std::ostringstream os;
        os << value;
        return os.str();
    }
}// namespace std
#endif

namespace Cicada {
    class AnalyticsQueryListener;

    class IAnalyticsCollector;

    class IAnalyticsCollectorFactory;

    typedef void (*CicadaLogCallback)(void *userData, int prio, const char *buf);


    class MediaPlayer {
    public:
        MediaPlayer();

        MediaPlayer(IAnalyticsCollectorFactory &factory);

        ~MediaPlayer();

        static string GetSdkVersion()
        {
            return "paas 0.9";//TODO version
        }

    public:
        void EnableVideoRenderedCallback(bool enable);

        void SetOnRenderFrameCallback(onRenderFrame cb, void *userData);

        void SetStreamTypeFlags(uint64_t flags);

        /*
         * set player listener
         */
        void SetListener(const playerListener &Listener);

        /*
         * enable hardware decoder or not
         */
        void EnableHardwareDecoder(bool bEnabled);

        /*
         * set player view
         */
        void SetView(void *view);

        /*
         * set player url source
         */
        void SetDataSource(const char *url);

        /*
        * when play url not equals SetDataSource , will callback.
        */
        void SetDataSourceChangedCallback(function<void(const string &)> urlChangedCallbak);

        /*
         *select specific track info
         */
        void SelectTrack(int index);

        /*
         * prepare the url
         */
        void Prepare();

        /*
         * start to play
         */
        void Start();

        /*
         * Pause the player
         */
        void Pause();

        /*
         * set the volume
         */
        void SetVolume(float volume);

        /**
         * get current volume of the player
         */
        float GetVolume();

        /*
         * seekt to new Postion by seekmode
         */
        void SeekTo(int64_t seekPos, SeekMode mode);

        /*
         * capture screen, buffer will get by callback
         */
        void CaptureScreen();

        /*
         * stop playing
         */
        void Stop();

        /*
         * get the total duration of playing url
         */
        int64_t GetDuration();

        /*
        * get master clock pts
        */
        int64_t GetMasterClockPts();//TODO ??这是什么？


        void SetClockRefer(clockRefer cb, void *arg);

        /*
         * get current stream index
         */
        int GetCurrentStreamIndex(StreamType type);//TODO ??这是什么？

        /*
         * get current stream info
         */
        StreamInfo *GetCurrentStreamInfo(StreamType type);

        /*
         * get the current playing position of the player
         */
        int64_t GetCurrentPosition();

        /*
         * get the buffered postion of the player
         */
        int64_t GetBufferedPosition();

        /*
         * enter background if back is true, or enter foreground
         *  this function is only used for ios?
         */
        void EnterBackGround(bool back);

        /*
         * set the player mute
         */
        void SetMute(bool bMute);

        /**
         * return ismuted or not
         */
        bool IsMuted();

        /**
         * set player config
         */
        void SetConfig(const MediaPlayerConfig *config);

        /**
         * get playerg config
         */
        const MediaPlayerConfig *GetConfig();

        /*
         * set Scale Mode
         */
        void SetScaleMode(ScaleMode mode);

        /*
         * Get Scale Mode
         */
        ScaleMode GetScaleMode();

        /*
         * set loop
         */
        void SetLoop(bool bLoop);

        /*
         * get loop or not
         */
        bool IsLoop();

        /*
         * get video width of current playing
         */
        int GetVideoWidth();

        /*
         * get video height of current playing
         */
        int GetVideoHeight();

        /*
         * get video rotation angle from video metadata
         */
        int GetVideoRotation();

        /*
         * set rotate mode, refer to RotateMode
         */
        void SetRotateMode(RotateMode mode);

        /*
         * get rotate mode
         */
        RotateMode GetRotateMode();

        /*
         * set mirror mode, refer to MirrorMode
         */
        void SetMirrorMode(MirrorMode mode);

        /*
         * get mirror mode
         */
        MirrorMode GetMirrorMode();

        /*
         * set clear color
         */
        void SetVideoBackgroundColor(uint32_t color);

        /*
         * set speed of current playing, 0.5-2.0
         */
        void SetSpeed(float speed);

        /*
         * get the speed of current playing
         */
        float GetSpeed();

        /*
         * set traceID for debug log tracing
         */
        void SetTraceID(const char *traceID);

        /**
   * set auto play
   */
        void SetAutoPlay(bool bAutoPlay);

        /**
         * get is auto play or not
         */
        bool IsAutoPlay();

        void Reload();

        /*
         * set Component callback by type
         */
        void SetComponentCb(player_component_type type, void *factory);

        void setBitStreamCb(readCB read, seekCB seek, void *arg);

        void SetMediaFrameCb(playerMediaFrameCb func, void *arg);

        int GetCurrentStreamMeta(Stream_meta *meta, StreamType type);

        void AddExtSubtitle(const char *uri);

        void SelectExtSubtitle(int index, bool select);

        void setErrorConverter(ErrorConverter *converter);

        std::string GetPropertyString(PropertyKey key);

        void SetOption(const char *key, const char *value);

        void GetOption(const char *key, char *value);

        IAnalyticsCollector *GetAnalyticsCollector()
        {
            return mCollector;
        }
        //
        //        long ApsaraGetPropertyLong(playerHandle *player, int key);

        //        void SetMediaFrameCb(plyerMediaFrameCb func, void* arg);

        /**
         * set cache config.
         * @param config
         */
        void SetCacheConfig(const CacheConfig &config);

        /**
         * get cache url. Should SetCacheConfig before call this.
         * @param URL orignal url
         * @return cached url.
         */
        string GetCachePathByURL(const string &URL);

        void SetDefaultBandWidth(int bandWidth);

    protected:
        static void preparedCallback(void *userData);

        static void completionCallback(void *userData);

        static void firstFrameCallback(void *userData);

        static void errorFrameCallback(int64_t errorCode, const void *errorMsg, void *userData);

        static void eventCallback(int64_t code, const void *msg, void *userData);

        static void videoSizeChangedCallback(int64_t width, int64_t height, void *userData);

        static void videoRenderedCallback(int64_t timeMs, int64_t pts, void *userData);

        static void currentPostionCallback(int64_t position, void *userData);

        static void bufferPostionCallback(int64_t position, void *userData);

        static void loopingStartCallback(void *userData);

        static void loadingStartCallback(void *userData);

        static void loadingEndCallback(void *userData);

        static void loadingProgressCallback(int64_t prg, void *userData);

        static void subtitleShowCallback(int64_t index, int64_t size, const void *content, void *userData);

        static void subtitleHideCallback(int64_t index, int64_t size, const void *content, void *userData);

        static void subtitleExtAddedCallback(int64_t index, const void *url, void *userData);

        static void streamChangedSucCallback(int64_t type, const void *Info, void *userData);

        static void PlayerSeeking(int64_t seekInCache, void *userData);

        static void PlayerSeekEnd(int64_t seekInCache, void *userData);

        static void PlayerStatusChanged(int64_t oldStatus, int64_t newStatus, void *userData);

        static void streamInfoGetCallback(int64_t count, const void *Infos, void *userData);

        static void captureScreenResult(int64_t width, int64_t height, const void *buffer, void *userData);

        static void autoPlayStart(void *userData);

        void abrChanged(int stream);

        static void onMediaFrameCallback(void *arg, const unique_ptr<IAFPacket> &frame, StreamType type);
        void mediaFrameCallback(const unique_ptr<IAFPacket> &frame, StreamType type);

    private:
        void configPlayer(const MediaPlayerConfig *config) const;

        void dummyFunction(bool dummy);

    protected:
        void *mPlayerHandle = nullptr;
        playerListener mListener{nullptr};
        MediaPlayerConfig *mConfig;
        AnalyticsQueryListener *mQueryListener;
        IAnalyticsCollector *mCollector{nullptr};
        IAnalyticsCollectorFactory &mCollectorFactory;
        AbrManager *mAbrManager;
        AbrAlgoStrategy *mAbrAlgo;

        std::mutex mMutexAbr;

        bool mLoop = false;
        bool waitingForStart = false;
        bool waitingForLoop = false;
        string mPlayUrl;
        CacheManager *mCacheManager = nullptr;
        CacheConfig mCacheConfig;
        PlayerStatus mOldPlayStatus{PLAYER_IDLE};

        playerMediaFrameCb mMediaFrameFunc = nullptr;
        void *mMediaFrameArg = nullptr;

        function<void(const string &)> mPlayUrlChangedCallback = nullptr;
    };
}// namespace Cicada


#endif /* CicadaPlayer_h */
