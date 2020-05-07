//
// Created by moqi on 2018/9/26.
//

#define LOG_TAG "PlayerNotifier"
#include <utils/timer.h>
#include "player_notifier.h"
#include <type_traits>

#include <utils/frame_work_log.h>

using namespace std;

namespace Cicada {

#define ARG_FLAG(x) static const int ARG_TYPE_##x = 1 << x
    ARG_FLAG(1);
    ARG_FLAG(2);
    ARG_FLAG(3);
#define ARG_FLAGS_12  (ARG_TYPE_1 | ARG_TYPE_2)
#define ARG_FLAGS_13  (ARG_TYPE_1 | ARG_TYPE_3)
#define ARG_FLAGS_123 (ARG_FLAGS_12 | ARG_TYPE_3)

    static void releaseIPacket(void *data)
    {
        delete (IAFPacket *) data;
    }

    class player_event
    {
    private:
        typedef void (*releaseFunc)(void *data);
        union function {
            playerVoidCallback p0;
            playerType1Callback p1;
            playerType12Callback p12;
            playerType13Callback p13;
            playerType123Callback p123;

        };
    public:
        explicit player_event(playerVoidCallback func)
            : arg1(0),
              arg2(0),
              data(nullptr),
              argFlags(0)
        {
            mFunc.p0 = func;
        }

        player_event(int64_t _arg1, playerType1Callback func)
            : arg1(_arg1),
              arg2(0),
              data(nullptr),
              argFlags(ARG_TYPE_1)
        {
            mFunc.p1 = func;
        }

        player_event(int64_t _arg1, int64_t _arg2, playerType12Callback func)
            : arg1(_arg1),
              arg2(_arg2),
              data(nullptr),
              argFlags(ARG_FLAGS_12)
        {
            mFunc.p12 = func;
        }

        player_event(int64_t _arg1, void *_data, playerType13Callback func, bool keepData = false, releaseFunc release = nullptr)
            : arg1(_arg1),
              arg2(0),
              data(_data),
              argFlags(ARG_FLAGS_13),
              mKeepData(keepData),
              mRelease(release)
        {
            mFunc.p13 = func;
        }

        player_event(int64_t _arg1, int64_t _arg2, void *_data, playerType123Callback func, bool keepData = false,
                     releaseFunc release = nullptr)
            : arg1(_arg1),
              arg2(_arg2),
              data(_data),
              argFlags(ARG_FLAGS_123),
              mKeepData(keepData),
              mRelease(release)

        {
            mFunc.p123 = func;
        }

        void onEvent(void *userData)
        {
            switch (argFlags) {
                case 0: {
                    mFunc.p0(userData);
                    break;
                }

                case ARG_TYPE_1: {
                    mFunc.p1(arg1, userData);
                    break;
                }

                case ARG_FLAGS_12: {
                    mFunc.p12(arg1, arg2, userData);
                    break;
                }

                case ARG_FLAGS_13: {
                    mFunc.p13(arg1, data, userData);
                    break;
                }

                case ARG_FLAGS_123: {
                    mFunc.p123(arg1, arg2, data, userData);
                    break;
                }

                default:
                    break;
            }
        }

        ~player_event()
        {
            if (!mKeepData && data) {
                if (mRelease) {
                    mRelease(data);
                } else {
                    free(data);
                }
            }
        }

    public:
        function mFunc{nullptr};
        releaseFunc mRelease = nullptr;

    private:
        int64_t arg1{};
        int64_t arg2{};
        void *data;
        int argFlags;
        bool mKeepData = false;
    };

    PlayerNotifier::PlayerNotifier()
    {
        mpThread = NEW_AF_THREAD(post_loop);
    }

    PlayerNotifier::~PlayerNotifier()
    {
        {
            std::unique_lock<std::mutex> uMutex(mMutex);
            mRunning = false;
        }
        mCondition.notify_one();
        delete mpThread;
        Clean();
    }

    void PlayerNotifier::setListener(const playerListener &listener)
    {
        mpThread->pause();
        mListener = listener;
        mpThread->start();
    }

    void PlayerNotifier::NotifyCaptureScreen(uint8_t *buffer, int width, int height)
    {
        if (!mEnable || mListener.CaptureScreen == nullptr) {
            return;
        }

        auto *dupBuffer = static_cast<uint8_t *>(malloc(width * height * 4));
        memcpy(dupBuffer, buffer, width * height * 4);
        auto *event = new player_event(width, height, dupBuffer, mListener.CaptureScreen, false);
        pushEvent(event);
    }

    void PlayerNotifier::NotifyPosition(int64_t pos)
    {
        AF_LOGD("NotifyPosition() :%lld", pos);

        if (!mEnable || mListener.PositionUpdate == nullptr) {
            return;
        }

        auto *event = new player_event(pos, mListener.PositionUpdate);
        pushEvent(event);
    }

    void PlayerNotifier::NotifyBufferPosition(int64_t pos)
    {
        if (!mEnable || mListener.BufferPositionUpdate == nullptr) {
            return;
        }

        auto *event = new player_event(pos, mListener.BufferPositionUpdate);
        pushEvent(event);
    }

    void PlayerNotifier::NotifyVideoSizeChanged(int64_t width, int64_t height)
    {
        if (!mEnable || mListener.VideoSizeChanged == nullptr) {
            return;
        }

        auto *event = new player_event(width, height, mListener.VideoSizeChanged);
        pushEvent(event);
    }

    void PlayerNotifier::NotifyVideoRendered(int64_t timeMs, int64_t pts)
    {
        if (!mEnable || mListener.VideoRendered == nullptr) {
            return;
        }

        auto *event = new player_event(timeMs, pts, mListener.VideoRendered);
        pushEvent(event);
    }

    void PlayerNotifier::NotifyFirstFrame()
    {
        if (!mEnable || mListener.FirstFrameShow == nullptr) {
            return;
        }

        NotifyVoidEvent(mListener.FirstFrameShow);
    }

    void PlayerNotifier::NotifyStreamInfo(StreamInfo *info[], int size)
    {
        if (!mEnable || mListener.StreamInfoGet == nullptr) {
            return;
        }

        auto *event = new player_event(size, info, mListener.StreamInfoGet, true);
        pushEvent(event);
    }

    void PlayerNotifier::CancelNotifyStreamInfo()
    {
        if (mListener.StreamInfoGet == nullptr) {
            return;
        }

        std::unique_lock<std::mutex> uMutex(mMutex);

        for (auto &event : mEventQueue) {
            if (event->mFunc.p13 == mListener.StreamInfoGet) {
                mEventQueue.remove(event);
            }
        }
    }

    void PlayerNotifier::NotifySubtitleEvent(subTitle_event id, IAFPacket *packet, int64_t index, const char *url)
    {
        if (!mEnable) {
            return;
        }

        player_event *event = nullptr;

        if (id == subTitle_event_show) {
            if (mListener.SubtitleShow)
                event = new player_event(packet->getInfo().pts, (int64_t) (sizeof(IAFPacket)), (void *) (packet), mListener.SubtitleShow,
                                         true);
        } else if (id == subTitle_event_hide) {
            if (mListener.SubtitleHide)
                event = new player_event(packet->getInfo().pts, (int64_t) (sizeof(IAFPacket)), (void *) (packet), mListener.SubtitleHide,
                                         false, releaseIPacket);
        } else if (id == subTitle_event_ext_added) {
            if (mListener.SubtitleExtAdd) {
                event = new player_event(index, strdup(url), mListener.SubtitleExtAdd);
            }
        }

        if (event == nullptr) {
            return;
        }

        pushEvent(event);
    }

    void PlayerNotifier::NotifyEvent(int code, const char *desc)
    {
        if (!mEnable || mListener.EventCallback == nullptr) {
            return;
        }

        auto *event = new player_event(code, strdup(desc), mListener.EventCallback);
        pushEvent(event);
    }

    void PlayerNotifier::NotifyError(int code, const char *desc)
    {
        if (!mEnable || mListener.ErrorCallback == nullptr) {
            return;
        }

        auto *event = new player_event(code, strdup(desc), mListener.ErrorCallback);
        pushEvent(event);
    }

    void PlayerNotifier::NotifyCompletion()
    {
        if (!mEnable || mListener.Completion == nullptr) {
            return;
        }

        NotifyVoidEvent(mListener.Completion);
    }

    void PlayerNotifier::NotifyLoopStart()
    {
        if (!mEnable || mListener.LoopingStart == nullptr) {
            return;
        }

        NotifyVoidEvent(mListener.LoopingStart);
    }

    void PlayerNotifier::NotifyVoidEvent(playerVoidCallback listener)
    {
        if (!mEnable) {
            return;
        }

        auto *event = new player_event(listener);
        pushEvent(event);
    }

    void PlayerNotifier::NotifySeeking(bool seekInCache) {
        if (!mEnable) {
            return;
        }

        auto *event = new player_event(seekInCache ? 1 : 0 , mListener.Seeking);
        pushEvent(event);
    }

    void PlayerNotifier::NotifySeekEnd(bool seekInCache)
    {
        if (!mEnable || mListener.SeekEnd == nullptr) {
            return;
        }

        auto *event = new player_event(seekInCache ? 1 : 0, mListener.SeekEnd);
        pushEvent(event);
    }

    void PlayerNotifier::NotifyStreamChanged(StreamInfo *info, StreamType type)
    {
        if (!mEnable || mListener.StreamSwitchSuc == nullptr) {
            return;
        }

        auto *event = new player_event(type, info, mListener.StreamSwitchSuc, true);
        pushEvent(event);
    }

    void PlayerNotifier::NotifyPrepared()
    {
        if (!mEnable || mListener.Prepared == nullptr) {
            return;
        }

        NotifyVoidEvent(mListener.Prepared);
    }

    void PlayerNotifier::NotifyAutoPlayStart()
    {
        if (!mEnable  || mListener.AutoPlayStart == nullptr) {
            return;
        }

        NotifyVoidEvent(mListener.AutoPlayStart);
    }

    void PlayerNotifier::NotifyLoading(loading_event loadingEvent, int progress)
    {
        if (!mEnable) {
            return;
        }

        player_event *event = nullptr;

        if (loadingEvent == loading_event_start) {
            if (mListener.LoadingStart) {
                event = new player_event(mListener.LoadingStart);
            }
        } else if (loadingEvent == loading_event_end) {
            if (mListener.LoadingEnd) {
                event = new player_event(mListener.LoadingEnd);
            }
        } else {
            if (mListener.LoadingProgress) {
                event = new player_event(progress, mListener.LoadingProgress);
            }
        }

        if (event == nullptr) {
            return;
        }

        pushEvent(event);
    }

    void PlayerNotifier::pushEvent(player_event *event)
    {
        std::unique_lock<std::mutex> uMutex(mMutex);
        mEventQueue.push_back(unique_ptr<player_event>(event));
        mCondition.notify_one();
    }

    void PlayerNotifier::NotifyPlayerStatusChanged(PlayerStatus from, PlayerStatus to)
    {
        if (!mEnable || mListener.StatusChanged == nullptr) {
            return;
        }

        auto *event = new player_event(from, to, mListener.StatusChanged);
        pushEvent(event);
    }

    int PlayerNotifier::post_loop()
    {
        if (!mRunning) {
            return -1;
        }

        std::unique_ptr<player_event> playerEvent = nullptr;
        {
            std::unique_lock<std::mutex> uMutex(mMutex);

            if (mEventQueue.empty()) {
                mCondition.wait(uMutex, [this]() {
                    return !mRunning || !mEventQueue.empty();
                });

                if (mEventQueue.empty()) {
                    return 0;
                }
            }

            playerEvent = move(mEventQueue.front());
            mEventQueue.pop_front();
        }

        if (playerEvent != nullptr) {
            playerEvent->onEvent(mListener.userData);
        }

        return 0;
    }

    void PlayerNotifier::Enable(bool enable)
    {
        mEnable = enable;
    }

    void PlayerNotifier::Clean()
    {
        std::unique_lock<std::mutex> uMutex(mMutex);

        while (!mEventQueue.empty()) {
            mEventQueue.pop_front();
        }
    }
}
