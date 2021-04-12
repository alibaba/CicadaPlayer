//
// Created by yuyuan on 2021/03/17.
//

#include "DashManager.h"
#include "DashSegmentTracker.h"
#include "DashStream.h"
#include "demuxer/play_list/AdaptationSet.h"
#include "demuxer/play_list/Period.h"
#include "demuxer/play_list/Representation.h"
#include "demuxer/play_list/playList_demuxer.h"
#include "utils/errors/framework_error.h"
#include "utils/frame_work_log.h"
#include "utils/mediaFrame.h"
#include <cassert>
#include <cerrno>

#undef LOG_TAG
#define LOG_TAG "DashManager"

using namespace Cicada;

DashManager::DashManager(playList *pList) : PlaylistManager(pList)
{}

DashManager::~DashManager()
{
    for (auto i = mStreamInfoList.begin(); i != mStreamInfoList.end(); i++) {
        AF_TRACE;
        (*i)->mPFrame = nullptr;
        (*i)->mPStream->close();
        delete (*i)->mPStream;
        delete (*i);
    }

    mStreamInfoList.clear();
}

int DashManager::init()
{
    int ret;
    std::list<Period *> &periodList = mPList->GetPeriods();
    int id = 0;

    for (auto &pit : periodList) {
        std::list<AdaptationSet *> &adaptSetList = FindSuitableAdaptationSets(pit);

        for (auto &ait : adaptSetList) {
            auto representList = ait->getRepresentations();

            for (auto &rit : representList) {
                rit->mPlayListType = playList_demuxer::playList_type_dash;
                auto *pTracker = new DashSegmentTracker(ait, rit, mSourceConfig);
                pTracker->setOptions(mOpts);
                auto *info = new DashStreamInfo();
                info->mPStream = new DashStream(pTracker, id++);
                info->mPStream->setOptions(mOpts);
                info->mPStream->setDataSourceConfig(mSourceConfig);
                info->mPStream->setBitStreamFormat(mMergeVideoHeader, mMergerAudioHeader);
                mStreamInfoList.push_back(info);
            }
        }
    }

    if (mStreamInfoList.size() == 1) {
        ret = (*mStreamInfoList.begin())->mPStream->open();
        if (ret >= 0) {
            mMuxedStream = (*mStreamInfoList.begin())->mPStream;
            mMuxedStream->setExtDataSource(mExtDataSource);
        }
    }

    return 0;
}

void DashManager::stop()
{
    for (auto &i : mStreamInfoList) {
        if (i->mPStream->isOpened()) {
            i->mPStream->stop();
        }
    }

    mStarted = false;
}

int DashManager::GetNbStreams() const
{
    if (mMuxedStream) {
        return mMuxedStream->GetNbStreams();
    } else {
        return static_cast<int>(mStreamInfoList.size());
    }
}

int DashManager::GetStreamMeta(Stream_meta *meta, int index, bool sub) const
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
    DashStream *stream = nullptr;

    for (const auto &item : mStreamInfoList) {
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
        }

        return 0;
    }

    AF_LOGE("no such stream %d\n", index);
    return -1;
}

static const char *getStreamTypeName(DashStream *pStream)
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

int DashManager::ReadPacket(unique_ptr<IAFPacket> &packet, int index)
{
    IAFPacket *pFrameOut{};

    if (mMuxedStream) {//mediaPlayList
        int ret = mMuxedStream->read(packet);

        if (packet != nullptr) {
            packet->getInfo().streamIndex = GEN_STREAM_ID(mMuxedStream->getId(), packet->getInfo().streamIndex);
        }

        return ret;
    }

    int ret;

    for (auto &i : mStreamInfoList) {
        if (i->mPStream->isOpened() && i->selected && i->mPFrame == nullptr && !i->eos) {
            ret = i->mPStream->read(i->mPFrame);
            if (ret > 0) {
                i->mPFrame->getInfo().streamIndex = GEN_STREAM_ID(i->mPStream->getId(), i->mPFrame->getInfo().streamIndex);
            } else if (ret == 0) {
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
                                AF_LOGE("set SegNum to %llu\n", i->mPStream->getCurSegNum() + 1);
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
            if (pFrameOut == i->mPFrame.get()) {
                packet = move(i->mPFrame);
                break;
            }
        }
    }

    if (pFrameOut == nullptr || pFrameOut->getSize() == 0) {
        AF_LOGD("EOS");
        return 0;
    }

    return packet->getSize();
}

int DashManager::OpenStream(int index)
{
    int ret = 0;
    AF_LOGD("OpenStream %d\n", index);

    // select stream
    if (mStreamInfoList.size() == 1) {
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
                i->mPStream->start();
                break;
            }
        }
    }

    return ret;
}

const std::string DashManager::GetProperty(int index, const string &key)
{
    for (auto &i : mStreamInfoList) {
        if (i->mPStream->getId() == index) {
            return i->mPStream->GetProperty(key);
        }
    }

    return "";
}

int DashManager::GetRemainSegmentCount(int index)
{
    for (auto &i : mStreamInfoList) {
        if (i->mPStream->getId() == index) {
            return i->mPStream->GetRemainSegmentCount();
        }
    }

    return -1;
}

void DashManager::CloseStream(int id)
{
    AF_LOGD("CloseStream %d\n", id);

    if (mMuxedStream) {
        return;
    }

    for (auto &i : mStreamInfoList) {
        if (i->mPStream->getId() == id) {
            if (!i->selected) {
                AF_LOGW("CloseStream not opened\n");
            }

            i->selected = false;
            i->mPStream->stop();
            i->mPFrame = nullptr;
            break;
        }
    }
}

int DashManager::start()
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

int64_t DashManager::seek(int64_t us, int flags, int index)
{
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
            }
        }

        // 2. seek video first ,get the seekedUs
        type = STREAM_TYPE_VIDEO;

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

int DashManager::SwitchStreamAligned(int from, int to)
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

int DashManager::getNBSubStream(int index) const
{
    for (const auto &i : mStreamInfoList) {
        if (i->mPStream->getId() == index) {
            return i->mPStream->getNBStream();
        }
    }

    return 0;
}

void DashManager::interrupt(int inter)
{
    for (auto &i : mStreamInfoList) {
        i->mPStream->interrupt(inter);
    }
}

bool DashManager::isRealTimeStream(int index)
{
    for (auto &i : mStreamInfoList) {
        if (i->mPStream->getId() == index) {
            return i->mPStream->isRealTimeStream();
        }
    }

    return false;
}

bool DashManager::isWallclockTimeSyncStream(int index)
{
    if (mPList == nullptr) {
        return false;
    }
    return mPList->isLive();
}

int64_t DashManager::getTargetDuration()
{
    if (mPList == nullptr) {
        return 0;
    }
    return mPList->maxSegmentDuration;
}

std::list<AdaptationSet *> DashManager::FindSuitableAdaptationSets(Period* period)
{
    std::list<AdaptationSet *> &adaptSetList = period->GetAdaptSets();
    AdaptationSet *suitableVideo = nullptr;
    AdaptationSet *suitableAudio = nullptr;

    for (auto &ait : adaptSetList) {
        auto representList = ait->getRepresentations();
        std::string mimeType = ait->getMimeType();
        if (mimeType.empty()) {
            for (auto &rit : representList) {
                mimeType = rit->getMimeType();
                if (!mimeType.empty()) {
                    break;
                }
            }
        }
        if (mimeType == "video/mp4") {
            if (suitableVideo) {
                continue;
            }
            suitableVideo = ait;
        } else if (mimeType == "audio/mp4") {
            if (suitableAudio) {
                continue;
            }
            suitableAudio = ait;
        }
        // TODO: subtitle
    }
    std::list<AdaptationSet *> ret;
    if (suitableVideo) {
        ret.push_back(suitableVideo);
    }
    if (suitableAudio) {
        ret.push_back(suitableAudio);
    }
    return ret;
}
