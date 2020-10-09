//
// Created by moqi on 2019/11/1.
//

#ifndef CICADAPLAYERSDK_MEDIAPLAYERSUBTITLELISTENER_H
#define CICADAPLAYERSDK_MEDIAPLAYERSUBTITLELISTENER_H

#include "player_notifier.h"
#include "subTitle/subTitlePlayer.h"
#include <utils/errors/framework_error.h>
#include <utils/frame_work_log.h>

namespace Cicada {
    class mediaPlayerSubTitleListener : public subTitlePlayer::Listener {
    public:
        explicit mediaPlayerSubTitleListener(PlayerNotifier &notifier) : mNotifier(notifier)
        {

        }

        ~mediaPlayerSubTitleListener() override = default;

        void onRender(bool show, IAFPacket *packet) override
        {
            //  AF_LOGD("%s [%s]\n", show ? "show" : "stop", data);
            if (show)
                mNotifier.NotifySubtitleEvent(subTitle_event_show, packet, 0, nullptr);
            else
                mNotifier.NotifySubtitleEvent(subTitle_event_hide, packet, 0, nullptr);
        };

        void onAdded(const std::string &uri, int index) override
        {
            if (index < 0) {
                AF_LOGE("add subtitle %s error %d(%s)\n", uri.c_str(), index, framework_err2_string(index));
            } else {
                AF_LOGD("add %s subtitle ok %d\n", uri.c_str(), index);
            }
            mNotifier.NotifySubtitleEvent(subTitle_event_ext_added, nullptr, index, uri.c_str());
        };

    private:
        PlayerNotifier &mNotifier;
    };
}


#endif //CICADAPLAYERSDK_MEDIAPLAYERSUBTITLELISTENER_H
