#include <utils/frame_work_log.h>
#include <cassert>
#include <utils/timer.h>
#include "player_msg_control.h"

namespace Cicada {

#define REPLACE_NONE 0
#define REPLACE_ALL -1
#define REPLACE_LAST  -2
#define SEEK_REPEAT_TIME 500 //ms
#define ADD_LOCK std::lock_guard<std::mutex> uMutex(mMutex)

    static int getRepeatTimeMS(PlayMsgType type)
    {
        switch (type) {
            case MSG_SETDATASOURCE:
            case MSG_SETVIEW:
            case MSG_PREPARE:
            case MSG_CHANGE_VIDEO_STREAM:
            case MSG_CHANGE_AUDIO_STREAM:
            case MSG_CHANGE_SUBTITLE_STREAM:
            case MSG_MUTE:
            case MSG_SET_DISPLAY_MODE:
            case MSG_SET_ROTATE_MODE:
            case MSG_SET_MIRROR_MODE:
            case MSG_SET_VIDEO_BACKGROUND_COLOR:
                return REPLACE_ALL;

            case MSG_START:
            case MSG_PAUSE:
            case MSG_SET_SPEED:
                return REPLACE_LAST;

            case MSG_SEEKTO:
                return SEEK_REPEAT_TIME;

            case MSG_INTERNAL_VIDEO_RENDERED:
            case MSG_ADD_EXT_SUBTITLE:
            case MSG_SELECT_EXT_SUBTITLE:
            case MSG_INTERNAL_VIDEO_CLEAN_FRAME:
            case MSG_INTERNAL_VIDEO_HOLD_ON:
                return REPLACE_NONE;

            default:
                assert(0);
                return REPLACE_NONE;
        }
    }


    PlayerMessageControl::PlayerMessageControl(PlayerMessageControllerListener &processor)
        : mProcessor(processor)
    {
    }

    PlayerMessageControl::~PlayerMessageControl()
    {
        clear();
    }

    void PlayerMessageControl::recycleMsg(QueueMsgStruct &msg)
    {
        if (MSG_SETDATASOURCE == msg.msgType
                || MSG_ADD_EXT_SUBTITLE == msg.msgType
           ) {
            delete msg.msgParam.dataSourceParam.url;
            msg.msgParam.dataSourceParam.url = nullptr;
        }
    }

    void PlayerMessageControl::clear()
    {
        ADD_LOCK;

        for (auto &i : mMsgQueue) {
            recycleMsg(i);
        }

        mMsgQueue.clear();
    }

    void PlayerMessageControl::putMsg(PlayMsgType type, const MsgParam &msgContent)
    {
        QueueMsgStruct qm;
        qm.msgType = type;
        qm.msgParam = msgContent;
        qm.msgTime = af_gettime_relative() / 1000;
        ADD_LOCK;
        int repeatTime = getRepeatTimeMS(type);

        switch (repeatTime) {
            case REPLACE_ALL:
                for (auto it = mMsgQueue.begin(); it != mMsgQueue.end();) {
                    if (it->msgType == type) {
                        recycleMsg(*it);
                        it = mMsgQueue.erase(it);
                    } else {
                        ++it;
                    }
                }

                break;

            case REPLACE_LAST:
                if (!mMsgQueue.empty() && mMsgQueue.back().msgType == type) {
                    recycleMsg(mMsgQueue.back());
                    mMsgQueue.pop_back();
                }

                break;

            case REPLACE_NONE:
            default:
                assert(repeatTime >= 0);

                if (0 < repeatTime) {
                    int count = 0;
                    auto first = mMsgQueue.end();
                    auto last = mMsgQueue.end();

                    for (auto it = mMsgQueue.begin(); it != mMsgQueue.end(); ++it) {
                        if (it->msgType == type) {
                            ++count;

                            if (mMsgQueue.end() == first) {
                                first = it;
                            }

                            last = it;
                        }
                    }

                    if (mMsgQueue.end() != last) {
                        if (qm.msgTime - last->msgTime < repeatTime) {
                            recycleMsg(*last);
                            mMsgQueue.erase(last);
                        }
                    }

                    if (2 <= count) {
                        recycleMsg(*first);
                        mMsgQueue.erase(first);
                    }
                }

                break;
        }

        mMsgQueue.push_back(qm);
    }

    int PlayerMessageControl::processMsg()
    {
        std::deque<QueueMsgStruct> processQueue;
        mMutex.lock();

        for (auto it = mMsgQueue.begin(); it != mMsgQueue.end();) {
            if (!mProcessor.OnPlayerMsgIsPadding(it->msgType, it->msgParam)) {
                processQueue.push_back(*it);
                it = mMsgQueue.erase(it);// don't need recycleMsg due to put to processQueue
            } else {
                ++it;
            }
        }

        mMutex.unlock();
        int count = 0;

        for (auto &it : processQueue) {
            OnPlayerMsgProcessor(it.msgType, it.msgParam);
            recycleMsg(it);

            if (MSG_INTERNAL_VIDEO_RENDERED != it.msgType) {
                count++;
            }
        }

        processQueue.clear();
        return count;
    }

    bool PlayerMessageControl::empty()
    {
        ADD_LOCK;
        return mMsgQueue.empty();
    }

    bool PlayerMessageControl::findMsgByType(PlayMsgType type)
    {
        ADD_LOCK;

        for (auto &i : mMsgQueue) {
            if (i.msgType == type) {
                return true;
            }
        }

        return false;
    }

    bool PlayerMessageControl::OnPlayerMsgProcessor(PlayMsgType msg, MsgParam msgContent)
    {
        switch (msg) {
            case MSG_PREPARE:
                mProcessor.ProcessPrepareMsg();
                break;

            case MSG_START:
                mProcessor.ProcessStartMsg();
                break;

            case MSG_PAUSE:
                mProcessor.ProcessPauseMsg();
                break;

            case MSG_SETDATASOURCE:
                mProcessor.ProcessSetDataSourceMsg(*(msgContent.dataSourceParam.url));
                break;

            case MSG_SETVIEW:
                mProcessor.ProcessSetViewMsg(msgContent.viewParam.view);
                break;

            case MSG_SET_DISPLAY_MODE:
                mProcessor.ProcessSetDisplayMode();
                break;

            case MSG_SET_ROTATE_MODE:
                mProcessor.ProcessSetRotationMode();
                break;

            case MSG_SET_MIRROR_MODE:
                mProcessor.ProcessSetMirrorMode();
                break;

            case MSG_SET_VIDEO_BACKGROUND_COLOR:
                mProcessor.ProcessSetVideoBackgroundColor();
                break;

            case MSG_SEEKTO:
                mProcessor.ProcessSeekToMsg(msgContent.seekParam.seekPos, msgContent.seekParam.bAccurate);
                break;

            case MSG_MUTE:
                mProcessor.ProcessMuteMsg();
                break;

            case MSG_CHANGE_VIDEO_STREAM:
                //    assert(mWillChangedVideoStreamIndex < 0);
                mProcessor.ProcessSwitchStreamMsg(msgContent.streamParam.index);
                break;

            case MSG_CHANGE_AUDIO_STREAM:
                //  assert(mWillChangedAudioStreamIndex < 0);
                mProcessor.ProcessSwitchStreamMsg(msgContent.streamParam.index);
                break;

            case MSG_CHANGE_SUBTITLE_STREAM:
                //   assert(mWillChangedSubtitleStreamIndex < 0);
                mProcessor.ProcessSwitchStreamMsg(msgContent.streamParam.index);
                break;

            case MSG_INTERNAL_VIDEO_RENDERED:
                mProcessor.ProcessVideoRenderedMsg(msgContent.videoRenderedParam.pts,
                                                   msgContent.videoRenderedParam.timeMs,
                                                   msgContent.videoRenderedParam.userData);
                break;

            case MSG_INTERNAL_VIDEO_CLEAN_FRAME:
                mProcessor.ProcessVideoCleanFrameMsg();
                break;

            case MSG_INTERNAL_VIDEO_HOLD_ON:
                mProcessor.ProcessVideoHoldMsg(msgContent.msgHoldOnVideoParam.hold);
                break;

            case MSG_ADD_EXT_SUBTITLE:
                mProcessor.ProcessAddExtSubtitleMsg(*msgContent.dataSourceParam.url);
                break;

            case MSG_SELECT_EXT_SUBTITLE:
                mProcessor.ProcessSelectExtSubtitleMsg(msgContent.msgSelectExtSubtitleParam.index,
                                                       msgContent.msgSelectExtSubtitleParam.bSelect);
                break;

            case MSG_SET_SPEED:
                mProcessor.ProcessSetSpeed(msgContent.msgSpeedParam.speed);
                break;

            default:
                AF_LOGE("Unknown msg\n");
                break;
        }

        return true;
    }

}//namespace Cicada
