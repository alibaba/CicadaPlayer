//
// Created by pingkai on 2020/12/28.
//

#ifndef CICADAMEDIA_SMPMESSAGECONTROLLERLISTENER_H
#define CICADAMEDIA_SMPMESSAGECONTROLLERLISTENER_H

#include "player_msg_control.h"
#include <utils/AFMediaType.h>
namespace Cicada {
    class SuperMediaPlayer;
    class SMPMessageControllerListener : public PlayerMessageControllerListener {
    public:
        explicit SMPMessageControllerListener(SuperMediaPlayer &player);
        ~SMPMessageControllerListener();

    public:
        void ProcessSetViewMsg(void *view) final;
        void ProcessSetSpeed(float speed) final;
        void ProcessVideoRenderedMsg(int64_t pts, int64_t timeMs, bool rendered, void *picUserData) final;
        void ProcessSeekToMsg(int64_t seekPos, bool bAccurate) final;
        void ProcessMuteMsg() final;
        void ProcessVideoHoldMsg(bool hold) final;

    private:
        bool OnPlayerMsgIsPadding(PlayMsgType msg, MsgParam msgContent) final;

        void ProcessPrepareMsg() final;

        void ProcessStartMsg() final;

        void ProcessSetDisplayMode() final;

        void ProcessSetRotationMode() final;

        void ProcessSetMirrorMode() final;

        void ProcessSetVideoBackgroundColor() final;

        void ProcessSetDataSourceMsg(const std::string &url) final;

        void ProcessPauseMsg() final;

        void ProcessSwitchStreamMsg(int index) final;

        void ProcessVideoCleanFrameMsg() final;

        void ProcessAddExtSubtitleMsg(const std::string &url) final;

        void ProcessSelectExtSubtitleMsg(int index, bool select) final;


    private:
        void switchVideoStream(int index, Stream_type type);
        void switchAudio(int index);
        void switchSubTitle(int index);
        int openUrl();

    private:
        SuperMediaPlayer &mPlayer;
    };
}// namespace Cicada


#endif//CICADAMEDIA_SMPMESSAGECONTROLLERLISTENER_H
