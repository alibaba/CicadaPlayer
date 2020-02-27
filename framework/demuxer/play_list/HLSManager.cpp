//
// Created by moqi on 2018/4/27.
//

#include "HLSManager.h"
#include "playList_demuxer.h"

#undef LOG_TAG
#define  LOG_TAG "HLSManager"

#include "../../utils/frame_work_log.h"
#include "../../utils/mediaFrame.h"
#include <cerrno>
#include <utils/errors/framework_error.h>
#include <cassert>

namespace Cicada {

    HLSManager::HLSManager(playList *pList)
        : PlaylistManager(pList)
    {
    }

    HLSManager::~HLSManager()
    {
        for (auto i = mStreamInfoList.begin(); i != mStreamInfoList.end(); i++) {
            AF_TRACE;
            (*i)->mPFrame = nullptr;
            (*i)->mPStream->close();
            delete (*i)->mPStream;
            //           mStreamInfoList.erase(i++);
            delete (*i);
        }

        mStreamInfoList.clear();
    }

    int HLSManager::init()
    {
        int ret;
        std::list<Period *> &periodList = mPList->GetPeriods();
        int id = 0;

        for (auto &pit : periodList) {
            std::list<AdaptationSet *> &adaptSetList = pit->GetAdaptSets();

            for (auto &ait : adaptSetList) {
                auto representList = ait->getRepresentations();

                for (auto &rit : representList) {
                    rit->mPlayListType = playList_demuxer::playList_type_hls;
                    auto *pTracker = new SegmentTracker(rit, mSourceConfig);
                    pTracker->setOptions(mOpts);
                    auto *info = new HLSStreamInfo();
                    info->mPStream = new HLSStream(pTracker, id++);
                    info->mPStream->setOptions(mOpts);
                    info->mPStream->setDataSourceConfig(mSourceConfig);
                    info->mPStream->setBitStreamFormat(mMergeVideoHeader, mMergerAudioHeader);
                    mStreamInfoList.push_back(info);
                }
            }
        }

        if (mStreamInfoList.size() == 1) {
            // mediaPlayList
            ret = (*mStreamInfoList.begin())->mPStream->open();

            if (ret >= 0) {
                mMuxedStream = (*mStreamInfoList.begin())->mPStream;
                mMuxedStream->setExtDataSource(mExtDataSource);
            }

            //      mMuxedStream->start();
        }

        return 0;
    }

    void HLSManager::stop()
    {
        for (auto &i : mStreamInfoList) {
            if (i->mPStream->isOpened()) {
                i->mPStream->stop();
            }
        }

        mStarted = false;
    }

    int HLSManager::GetNbStreams()
    {
        if (mMuxedStream) {
            return mMuxedStream->GetNbStreams();
        } else {// masterPlayList only support one es stream per stream url(ext-media-info ext-stream-info) now
            return static_cast<int>(mStreamInfoList.size());
        }
    }

    int HLSManager::GetStreamMeta(Stream_meta *meta, int index, bool sub)
    {
        memset(meta, 0, sizeof(Stream_meta));
        meta->type = STREAM_TYPE_UNKNOWN;
        int subIndex = GEN_SUB_STREAM_ID(index);
        int streamIndex = GEN_STREAM_INDEX(index);

        if (mMuxedStream) {
            if (mMuxedStream->getId() == streamIndex) {
                meta->type = STREAM_TYPE_MIXED;
                return mMuxedStream->GetStreamMeta(meta, subIndex, sub);
            } else {
                return -1;
            }
        }

        int i = 0;
        HLSStream *stream = nullptr;

        for (auto &item : mStreamInfoList) {
            if (i == streamIndex) {
                stream = item->mPStream;
                break;
            }

            i++;
        }

        if (stream) {
            stream->GetStreamMeta(meta, subIndex, sub);

            if (meta->type == STREAM_TYPE_UNKNOWN) {
                meta->type = static_cast<Stream_type>(stream->getStreamType());
                //   meta->bandwidth = stream->
            }

            return 0;
        }

        AF_LOGE("no such stream %d\n", index);
        return -1;
    }

    static const char *getStreamTypeName(HLSStream *pStream)
    {
        char *name = const_cast<char *>("unknown");

        switch (pStream->getStreamType()) {
            case STREAM_TYPE_VIDEO:
                name = const_cast<char *>("Video");
                break;

            case STREAM_TYPE_AUDIO:
                name = const_cast<char *>("Audio");
                break;

            case STREAM_TYPE_SUB:
                name = const_cast<char *>("Subtitle");
                break;

            default:
                break;
        }

        return name;
    }

    int HLSManager::ReadPacket(unique_ptr<IAFPacket> &packet, int index)
    {
        IAFPacket *pFrameOut{};

        if (mMuxedStream) { //mediaPlayList
            int ret = mMuxedStream->read(packet);

            if (packet != nullptr) {
                packet->getInfo().streamIndex = GEN_STREAM_ID(mMuxedStream->getId(), packet->getInfo().streamIndex);
            }

            return ret;
        }

        // TODO: detect eos
        int ret;

        for (auto &i : mStreamInfoList) {
            if (i->mPStream->isOpened() && i->selected && i->mPFrame == nullptr && !i->eos) {
                ret = i->mPStream->read(i->mPFrame);

                // AF_LOGD("CurSegNum is %llu", i.mPStream->getCurSegNum());
                if (ret > 0) {
//                    i.mPFrame->streamIndex = i->mPStream->getId();
                    // subId *100 + streamID
                    i->mPFrame->getInfo().streamIndex = GEN_STREAM_ID(i->mPStream->getId(),
                                                        i->mPFrame->getInfo().streamIndex);
                } else if (ret == 0) {
                    // TODO: don't block here
                    AF_LOGD("EOF %d\n", i->mPStream->getId());

                    if (i->stopOnSegEnd) {
                        i->mPStream->stop();
                        i->selected = false;

                        for (auto &j : mStreamInfoList) {
                            if (j->mPStream->getId() == i->toStreamId) {
                                j->selected = true;
                                j->stopOnSegEnd = false;
                                j->toStreamId = -1;

                                if (i->mPStream->isLive()) {
                                    uint64_t targetPosition = i->mPStream->getCurSegPosition() + 1;
                                    AF_LOGE("set SegPosition to %llu\n", targetPosition);
                                    j->mPStream->setCurSegPosition(targetPosition);
                                } else {
                                    AF_LOGE("set SegNum to %llu\n",
                                            i->mPStream->getCurSegNum() + 1);
                                    j->mPStream->SetCurSegNum(i->mPStream->getCurSegNum() + 1);
                                }

                                break;
                            }
                        }

                        i->stopOnSegEnd = false;
                        i->mPStream->stopOnSegEnd(false);
                        OpenStream(i->toStreamId);
                        AF_LOGD("change stream %d -> %d", i->mPStream->getId(), i->toStreamId);
                        i->toStreamId = -1;
                        return -EAGAIN;
                    } else {
                        i->eos = true;
                    }
                } else {
                    if (ret == FRAMEWORK_ERR_FORMAT_NOT_SUPPORT) {
                        AF_LOGE("read error %s\n", framework_err2_string(ret));
                        i->eos = true;
                        i->mPStream->stop();
                    } else if (ret != -EAGAIN) {
                        AF_LOGE("read error %d\n", ret);
                    }

                    return ret;
                }
            }

            if (i->mPFrame) {
                if (pFrameOut == nullptr) {
                    pFrameOut = i->mPFrame.get();
                } else if (i->mPFrame->getInfo().dts < pFrameOut->getInfo().dts) {
                    pFrameOut = i->mPFrame.get();
                }
            }
        }

        if (index != -1) {
            pFrameOut = nullptr;

            for (auto &i : mStreamInfoList) {
                if (i->mPStream->isOpened() && i->selected && i->mPFrame == nullptr && !i->eos) {
                    if (i->mPFrame->getInfo().streamIndex == index) {
                        pFrameOut = i->mPFrame.get();
                        packet = move(i->mPFrame);
                        break;
                    }
                }
            }
        } else {
            for (auto &i : mStreamInfoList) {
                if (i->mPFrame) {
//                    if (i->mPStream->getStreamType() == STREAM_TYPE_SUB) {
//                        AF_LOGD("(%s)i->mPFrame->streamIndex is %x, pts is %lld %s\n", getStreamTypeName(i->mPStream),
//                                i->mPFrame->streamIndex,
//                                i->mPFrame->pts, i->mPFrame->pBuffer);
//                    } else
//                        AF_LOGD("(%s)i->mPFrame->streamIndex is %x, pts is %lld\n", getStreamTypeName(i->mPStream),
//                                i->mPFrame->streamIndex,
//                                i->mPFrame->pts);
                }

                if (pFrameOut == i->mPFrame.get()) {
                    packet = move(i->mPFrame);
                    break;
                }
            }

//            AF_LOGD("\n");
        }

        if (pFrameOut == nullptr || pFrameOut->getSize() == 0) {
            AF_LOGD("EOS");
            return 0;
        }

        return packet->getSize();
    }

    int HLSManager::OpenStream(int index)
    {
        int ret = 0;
        AF_LOGD("OpenStream %d\n", index);

        // select stream
        if (mStreamInfoList.size() == 1) {
            // TODO: select stream
            // mediaPlayList
            if (!(*mStreamInfoList.begin())->mPStream->isOpened()) {
                ret = (*mStreamInfoList.begin())->mPStream->open();

                if (ret >= 0) {
                    mMuxedStream = (*mStreamInfoList.begin())->mPStream;
                }

                mMuxedStream->start();
            }
        } else {
            for (auto &i : mStreamInfoList) {
                if (i->mPStream->getId() == index) {
                    if (!i->mPStream->isOpened()) {
                        if (mFirstSeekPos != INT64_MIN) {
                            i->mPStream->seek(mFirstSeekPos, 0);
                        }

                        ret = i->mPStream->open();
                    }

                    i->selected = true;
                    //    if (mStarted)
                    i->mPStream->start();
                    break;
                }
            }
        }

        return ret;
    }

    const std::string HLSManager::GetProperty(int index, const string &key)
    {
        for (auto &i : mStreamInfoList) {
            if (i->mPStream->getId() == index) {
                return i->mPStream->GetProperty(key);
            }
        }

        return "";
    }

    int HLSManager::GetRemainSegmentCount(int index)
    {
        for (auto &i : mStreamInfoList) {
            if (i->mPStream->getId() == index) {
                return i->mPStream->GetRemainSegmentCount();
            }
        }

        return -1;
    }

    void HLSManager::CloseStream(int id)
    {
        AF_LOGD("CloseStream %d\n", id);

        if (mMuxedStream) {
            // TODO: select stream
            return;
        }

        for (auto &i : mStreamInfoList) {
            if (i->mPStream->getId() == id) {
                if (!i->selected) {
                    AF_LOGW("CloseStream not opened\n");
                }

                i->selected = false;
                // TODO: close the hlsStream? close at release
                //  if (mStarted)
                i->mPStream->stop();
                i->mPFrame = nullptr;
                break;
            }
        }
    }

    int HLSManager::start()
    {
        if (mMuxedStream) {
            if (mMuxedStream->isOpened()) {
                return mMuxedStream->start();
            }
        }

        for (auto &i : mStreamInfoList) {
            if (i->mPStream->isOpened() && i->selected) {
                i->mPStream->start();
            }
        }

        mStarted = true;
        return 0;
    }

    int HLSManager::seek(int64_t us, int flags, int index)
    {
        int ret = 0;
        bool hasOpened = false;

        for (auto &i : mStreamInfoList) {
            if (i->selected) {
                hasOpened = true;
                break;
            }
        }

        if (hasOpened) {
            mFirstSeekPos = INT64_MIN;
        } else {
            mFirstSeekPos = us;
        }

        if (mMuxedStream) {
            return mMuxedStream->seek(us, flags);
        }

        int type = STREAM_TYPE_UNKNOWN;

        if (index != -1) {
            for (auto &i : mStreamInfoList) {
                if (i->mPStream->getId() == index) {
                    type = i->mPStream->getStreamType();

                    if (type == STREAM_TYPE_SUB) {
                        return i->mPStream->seek(us, flags);
                    }

                    break;
                }
            }
        }

// seek all the stream
        if (index == -1) {
            // 1. finish all Aligned switch
            for (auto &i : mStreamInfoList) {
                if (i->stopOnSegEnd) {
                    CloseStream(i->mPStream->getId());

                    // TODO: open at seek finish
                    if (i->toStreamId >= 0) {
                        OpenStream(i->toStreamId);
                    }

                    i->toStreamId = -1;
                    i->stopOnSegEnd = false;
                    //      break;
                }
            }

            // 2. seek video first ,get the seekedUs
            type = STREAM_TYPE_VIDEO;
            // TODO type use bit or

            for (auto &i : mStreamInfoList) {
                if (i->selected) {
                    if (i->mPStream->getStreamType() == STREAM_TYPE_VIDEO || i->mPStream->getStreamType() == STREAM_TYPE_MIXED) {
                        type = i->mPStream->getStreamType();
                        int64_t seekedUs = i->mPStream->seek(us, flags);
                        AF_LOGD("first seeked time is %lld --> %lld", us, seekedUs);
                        us = seekedUs;
                        i->eos = false;
                        break;
                    }
                }
            }

            // 3. use the seekedUs to seek all other streams opened

            for (auto &i : mStreamInfoList) {
                if (i->selected && i->mPStream->getStreamType() != type) {
                    i->eos = false;
                    int64_t seekedUs = i->mPStream->seek(us, flags);

                    if (seekedUs < 0) {
                        return -1;
                    }

                    if (i->mPStream->getId() == index) {
                        break;
                    }

                    AF_LOGD("second seeked time is %lld --> %lld", us, seekedUs);
                }

                i->mPFrame = nullptr;
            }

            return 0;
        }

        if (type == STREAM_TYPE_UNKNOWN) {
            AF_LOGE("unknown stream type, can't seek");
            return -EINVAL;
        }

//     STREAM_TYPE_VIDEO, STREAM_TYPE_AUDIO, STREAM_TYPE_MIXED
        // 1. finish Aligned switch  TO ME

        for (auto &i : mStreamInfoList) {
            if (i->stopOnSegEnd && i->toStreamId == index) {
                CloseStream(i->mPStream->getId());

                // TODO: open at seek finish
                if (i->toStreamId >= 0) {
                    OpenStream(i->toStreamId);
                }

                i->toStreamId = -1;
                i->stopOnSegEnd = false;
                break;
            }
        }

// 2. do seek
        for (auto &i : mStreamInfoList) {
            if (i->mPStream->getId() == index) {
                i->eos = false;
                return i->mPStream->seek(us, flags);
            }
        }

        assert(0);
        AF_LOGE("unknown stream type, can't seek");
        return -EINVAL;
    }

    int HLSManager::SwitchStreamAligned(int from, int to)
    {
        AF_LOGD("SwitchStreamAligned %d -->%d\n", from, to);

        for (auto &i : mStreamInfoList) {
            if (i->mPStream->getId() == from) {
                // TODO: use seg start Time to Align the to stream seg num
                // TODO: deal when from is switching
                i->stopOnSegEnd = true;
                i->mPStream->stopOnSegEnd(true);
                i->toStreamId = to;
                break;
            }
        }

        return 0;
    }

    int HLSManager::getNBSubStream(int index)
    {
        for (auto &i : mStreamInfoList) {
            if (i->mPStream->getId() == index) {
                return i->mPStream->getNBStream();
            }
        }

        return 0;
    }

    void HLSManager::interrupt(int inter)
    {
        for (auto &i : mStreamInfoList) {
            i->mPStream->interrupt(inter);
        }
    }
}
