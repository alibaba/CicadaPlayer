//
// Created by moqi on 2019-05-29.
//


#define LOG_TAG "apsaraDataSourceListener"
#include <utils/timer.h>
#include <utils/frame_work_log.h>
#include "SuperMediaPlayerDataSourceListener.h"
#include "media_player_error_def.h"
#include "SuperMediaPlayer.h"
using namespace Cicada;

namespace Cicada {

    SuperMediaPlayerDataSourceListener::SuperMediaPlayerDataSourceListener(SuperMediaPlayer &player) :
        mPlayer(player),
        mNetworkConnected(false)
    {
        enableRetry_l();
    }

    IDataSource::Listener::NetWorkRetryStatus SuperMediaPlayerDataSourceListener::onNetWorkRetry(int error)
    {
        // TODO: use the error code to detect something, or report it out
        (void) error;
        std::lock_guard<std::mutex> uMutex(mMutex);
        mNetworkConnected = false;

        if (af_getsteady_ms() <= mEffectiveRetryTime) {
            return NetWorkRetryStatusRetry;
        }

        if (!bWaitingForRet) {
            mPlayer.mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_NETWORK_RETRY, "");
            AF_LOGD("MEDIA_PLAYER_EVENT_NETWORK_RETRY");
            bWaitingForRet = true;
        }

        return NetWorkRetryStatusPending;
    }

    void SuperMediaPlayerDataSourceListener::enableRetry()
    {
        std::lock_guard<std::mutex> uMutex(mMutex);
        enableRetry_l();
    }

    void SuperMediaPlayerDataSourceListener::enableRetry_l()
    {
        mEffectiveRetryTime = af_getsteady_ms() + mPlayer.mSet.timeout_ms;
        bWaitingForRet = false;
    }

    void SuperMediaPlayerDataSourceListener::onNetWorkConnected()
    {
        if (!mNetworkConnected) {
            AF_LOGD("onNetWorkRetry successful\n");
            mNetworkConnected = true;
            mPlayer.mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_NETWORK_RETRY_SUCCESS, "");
        }
    }

}
