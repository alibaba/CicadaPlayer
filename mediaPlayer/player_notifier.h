//
// Created by moqi on 2018/9/26.
//

#ifndef CICADA_PLAYER_PLAYER_NOTIFIER_H
#define CICADA_PLAYER_PLAYER_NOTIFIER_H

#include <list>
#include <mutex>
#include <condition_variable>
//#include "apsara_player_event_def.h"
#include "native_cicada_player_def.h"
#include "utils/afThread.h"

namespace Cicada {
    typedef enum loading_event_t {
        loading_event_start,
        loading_event_end,
        loading_event_progress,
    } loading_event;

    typedef enum subTitle_event_t {
        subTitle_event_hide,
        subTitle_event_show,
        subTitle_event_ext_added,
    } subTitle_event;

    class player_event;

    class PlayerNotifier {
    public:
        PlayerNotifier();

        ~PlayerNotifier();

        void Enable(bool enable);

        void Clean();

        void setListener(const playerListener &listener);

        void NotifyPosition(int64_t pos);

        void NotifyBufferPosition(int64_t pos);

        void NotifyVideoSizeChanged(int64_t width, int64_t height);

        void NotifyFirstFrame();

        void NotifyStreamInfo(StreamInfo *info[], int size);

        void NotifySubtitleEvent(subTitle_event id, IAFPacket *packet, int64_t index, const char *url);

        void NotifyError(int code, const char *desc);

        void NotifyEvent(int code, const char *desc);

        void NotifyCompletion();

        void NotifyLoopStart();

        void NotifySeekEnd(bool seekInCache);

        void NotifyPrepared();

        void NotifyCaptureScreen(uint8_t *buffer, int width, int height);

        // Still support change status when Enable as false
        void NotifyPlayerStatusChanged(PlayerStatus from, PlayerStatus to);

        void NotifyLoading(loading_event loadingEvent, int progress);

        void NotifyStreamChanged(StreamInfo *info, StreamType type);

        void CancelNotifyStreamInfo();

        void NotifyAutoPlayStart();

        void NotifyVideoRendered(int64_t timeMs, int64_t pts);

        void NotifySeeking(bool seekInCache);

    private:

        void NotifyVoidEvent(playerVoidCallback listener);

        void pushEvent(player_event *event);

        int post_loop();

    private:
        playerListener mListener{nullptr};
        std::list<std::unique_ptr<player_event>> mEventQueue;
        std::mutex mMutex;
        afThread *mpThread;
        std::condition_variable mCondition;
        bool mEnable = true;
        std::atomic_bool mRunning{true};
    };
}


#endif //CICADA_PLAYER_PLAYER_NOTIFIER_H
