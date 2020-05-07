#ifndef CICADA_PLAYER_MSG_CONTROL_H
#define CICADA_PLAYER_MSG_CONTROL_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include <string>

using namespace std;

namespace Cicada {
    typedef enum PlayMsgType {
        MSG_INVALID = -1,
        MSG_SETDATASOURCE = 0,
        MSG_SETVIEW,
        MSG_PREPARE,
        MSG_START,
        MSG_PAUSE,
        MSG_SEEKTO,
        MSG_MUTE,
        MSG_CHANGE_VIDEO_STREAM,
        MSG_CHANGE_AUDIO_STREAM,
        MSG_CHANGE_SUBTITLE_STREAM,

        MSG_SET_DISPLAY_MODE,
        MSG_SET_ROTATE_MODE,
        MSG_SET_MIRROR_MODE,
        MSG_SET_VIDEO_BACKGROUND_COLOR,

        MSG_ADD_EXT_SUBTITLE,
        MSG_SELECT_EXT_SUBTITLE,
        MSG_SET_SPEED,

        MSG_INTERNAL_VIDEO_FIRST = 0x100,
        MSG_INTERNAL_VIDEO_RENDERED = MSG_INTERNAL_VIDEO_FIRST,
        MSG_INTERNAL_VIDEO_CLEAN_FRAME,
        MSG_INTERNAL_VIDEO_HOLD_ON
    } PlayMsgType;


    typedef struct _DisplayModeParam {
        int mode;
    } DisplayModeParam;


    typedef struct _RotateModeParam {
        int mode;
    } RotateModeParam;

    typedef struct _MirrorModeParam {
        int mode;
    } MirrorModeParam;

    typedef struct _MsgViewParam {
        void *view;
    } MsgViewParam;

    typedef struct _MsgDataSourceParam {
        std::string *url;
    } MsgDataSourceParam;

    typedef struct _MsgSeekParam {
        int64_t seekPos;
        bool bAccurate;
    } MsgSeekParam;

    typedef struct _MsgSpeedParam {
        float speed;
    } MsgSpeedParam;

    typedef struct _MsgHoldOnVideoParam {

        bool hold;
    } MsgHoldOnVideoParam;

    typedef struct _MsgChangeStreamParam {
        int index;
    } MsgChangeStreamParam;

    typedef struct _MsgVideoRenderedParam {
        int64_t pts;
        int64_t timeMs;
        void *userData;
    } MsgVideoRenderedParam;

    typedef struct _MsgSelectExtSubtitleParam {
        int index;
        bool bSelect;
    } MsgSelectExtSubtitleParam;

    typedef union _MsgParam {
        MsgViewParam viewParam;
        MsgDataSourceParam dataSourceParam;
        MsgSeekParam seekParam;
        MsgChangeStreamParam streamParam;
        MsgVideoRenderedParam videoRenderedParam;
        MsgSelectExtSubtitleParam msgSelectExtSubtitleParam;
        MsgHoldOnVideoParam msgHoldOnVideoParam;
        MsgSpeedParam msgSpeedParam;
    } MsgParam;

    typedef struct _QueueMsgStruct {
        PlayMsgType msgType;
        MsgParam msgParam;
        int64_t msgTime;
    } QueueMsgStruct;

    typedef bool (*OnMsgProcesser)(PlayMsgType msg, MsgParam msgConent, void *userData);

    class PlayerMessageControllerListener {
    public:

        virtual ~PlayerMessageControllerListener() = default;

        virtual bool OnPlayerMsgIsPadding(PlayMsgType msg, MsgParam msgContent)
        {
            return false;
        };

        virtual void ProcessPrepareMsg() = 0;

        virtual void ProcessStartMsg() = 0;

        virtual void ProcessSetDisplayMode() = 0;

        virtual void ProcessSetRotationMode() = 0;

        virtual void ProcessSetMirrorMode() = 0;

        virtual void ProcessSetVideoBackgroundColor() = 0;

        virtual void ProcessSetViewMsg(void *view) = 0;

        virtual void ProcessSetDataSourceMsg(const std::string &url) = 0;

        virtual void ProcessPauseMsg() = 0;

        virtual void ProcessSeekToMsg(int64_t seekPos, bool bAccurate) = 0;

        virtual void ProcessMuteMsg() = 0;

        virtual void ProcessSwitchStreamMsg(int index) = 0;

        virtual void ProcessVideoRenderedMsg(int64_t pts, int64_t timeMs, void *picUserData) = 0;

        virtual void ProcessVideoCleanFrameMsg() = 0;

        virtual void ProcessVideoHoldMsg(bool hold) = 0;

        virtual void ProcessAddExtSubtitleMsg(const std::string &url) = 0;

        virtual void ProcessSelectExtSubtitleMsg(int index, bool select) = 0;

        virtual void ProcessSetSpeed(float speed) = 0;


    };

    class PlayerMessageControl {
    public:
        explicit PlayerMessageControl(PlayerMessageControllerListener &processor);

        ~PlayerMessageControl();

    public:
        void clear();

        void putMsg(PlayMsgType type, const MsgParam &msgContent);

        int processMsg();

        bool findMsgByType(PlayMsgType type);

        bool empty();

    private:

        void recycleMsg(QueueMsgStruct &msg);

        bool OnPlayerMsgProcessor(PlayMsgType msg, MsgParam msgContent);

        std::mutex mMutex;
        std::deque<QueueMsgStruct> mMsgQueue;
        //std::condition_variable     mCondition;

        PlayerMessageControllerListener &mProcessor;
    };

} // namespace Cicada
#endif // CICADA_PLAYER_MSG_CONTROL_H
