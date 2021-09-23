//
// Created by moqi on 2018/4/25.
//

#define  LOG_TAG "playList_demuxer"

#include "playList_demuxer.h"
#include "HLSManager.h"
#include "HlsParser.h"
#include "demuxer/dash/DashManager.h"
#include "demuxer/dash/MPDParser.h"

namespace Cicada {
    playList_demuxer playList_demuxer::se(0);

    playList_demuxer::playList_demuxer() = default;

    playList_demuxer::playList_demuxer(const string& path, playList_type type)
        : IDemuxer(path)
    {
        if (type == playList_type_hls) {
            mParser = new HlsParser(path.c_str());
        } else if (type == playList_type_dash) {
            mParser = new Dash::MPDParser(path);
        }
        mType = type;
    }

    playList_demuxer::~playList_demuxer()
    {
        delete mProxySource;
        delete mParser;
    }

    int playList_demuxer::Open()
    {
        int ret;

        if (!mParser) {
            return -1;
        }

        mProxySource = new proxyDataSource();
        mProxySource->setImpl(mReadCb, mSeekCb, mOpenCb, mInterruptCb, mSetSegmentList, mUserArg);
        mProxySource->setOptions(mOpts);
        PlaylistManager *playlistManager = nullptr;
        mParser->SetDataCallBack(mReadCb, mSeekCb, mUserArg);
        mPPlayList = mParser->parse(mPath);

        if (mPPlayList) {
            mPPlayList->dump();
        } else {
            return -EINVAL;
        }

        if (mType == playList_type_hls) {
            playlistManager = new HLSManager(mPPlayList);
        } else if (mType == playList_type_dash) {
            playlistManager = new DashManager(mPPlayList);
        }

        if (playlistManager == nullptr) {
            return -ENOMEM;
        }

        playlistManager->setOptions(mOpts);
        playlistManager->setExtDataSource(mProxySource);
        playlistManager->setDataSourceConfig(sourceConfig);
        playlistManager->setBitStreamFormat(mMergeVideoHeader, mMergeAudioHeader);
        mPPlaylistManager = playlistManager;
        ret = playlistManager->init();

        if (mFirstSeekPos != INT64_MIN) {
            playlistManager->seek(mFirstSeekPos, 0, -1);
        }

        return ret;
    }
    int playList_demuxer::ReadPacket(std::unique_ptr<IAFPacket> &packet, int index)
    {
        //  AF_LOGD(TAG,"ReadPacket\n");
        if (mPPlaylistManager) {
            return mPPlaylistManager->ReadPacket(packet, index);
        }

        return -EINVAL;
    }

    void playList_demuxer::Close()
    {
        delete mPPlaylistManager;
        delete mPPlayList;
        mPPlaylistManager = nullptr;
        mPPlayList = nullptr;
    }

    void playList_demuxer::Start()
    {
        if (mPPlaylistManager) {
            mPPlaylistManager->start();
        }
    }

    void playList_demuxer::Stop()
    {
        if (mPPlaylistManager) {
            mPPlaylistManager->stop();
        }
    }

    int64_t playList_demuxer::Seek(int64_t us, int flags, int index)
    {
        if (mPPlaylistManager) {
            return mPPlaylistManager->seek(us, flags, index);
        } else {
            mFirstSeekPos = us;
            return 0;
        }
    }

    int playList_demuxer::GetNbStreams() const
    {
        if (mPPlaylistManager) {
            return mPPlaylistManager->GetNbStreams();
        }

        return -EINVAL;
    }

    int playList_demuxer::GetSourceMeta(Source_meta **meta) const
    {
        return 0;
    }

    int playList_demuxer::GetMediaMeta(Media_meta *mediaMeta) const
    {
        return 0;
    }

    int playList_demuxer::GetStreamMeta(Stream_meta *meta, int index, bool sub) const
    {
        if (mPPlaylistManager) {
            return mPPlaylistManager->GetStreamMeta(meta, index, sub);
        }

        return -EINVAL;
    }

    int playList_demuxer::OpenStream(int index)
    {
        if (mPPlaylistManager) {
            return mPPlaylistManager->OpenStream(index);
        }

        return -EINVAL;
    }

    void playList_demuxer::CloseStream(int index)
    {
        if (mPPlaylistManager) {
            mPPlaylistManager->CloseStream(index);
        }
    }

    int playList_demuxer::GetRemainSegmentCount(int index)
    {
        if (mPPlaylistManager) {
            return mPPlaylistManager->GetRemainSegmentCount(index);
        }

        return -1;
    }

    const std::string playList_demuxer::GetProperty(int index, const string &key) const
    {
        if (mPPlaylistManager) {
            return mPPlaylistManager->GetProperty(index, key);
        }

        return "";
    }

    int playList_demuxer::SwitchStreamAligned(int from, int to)
    {
        if (mPPlaylistManager) {
            return mPPlaylistManager->SwitchStreamAligned(from, to);
        }

        return -EINVAL;
    }


    int playList_demuxer::GetNbSubStreams(int index) const
    {
        if (mPPlaylistManager) {
            return mPPlaylistManager->getNBSubStream(index);
        }

        return -EINVAL;
    }

    void playList_demuxer::interrupt(int inter)
    {
        if (mPPlaylistManager != nullptr) {
            mPPlaylistManager->interrupt(inter);
        }
    }

    bool playList_demuxer::isRealTimeStream(int index)
    {
        if (mPPlaylistManager) {
            return mPPlaylistManager->isRealTimeStream(index);
        }

        return false;
    }

    bool playList_demuxer::isWallclockTimeSyncStream(int index)
    {
        if (mPPlaylistManager) {
            return mPPlaylistManager->isWallclockTimeSyncStream(index);
        }

        return false;
    }

    int64_t playList_demuxer::getDurationToStartStream(int index)
    {
        if (mPPlaylistManager) {
            return mPPlaylistManager->getDurationToStartStream(index);
        }
        return 0;
    }

    int64_t playList_demuxer::getMaxGopTimeUs()
    {
        if (mPPlaylistManager) {
            return mPPlaylistManager->getTargetDuration();
        }
        return INT64_MIN;
    }

    vector<mediaSegmentListEntry> playList_demuxer::getSegmentList(int index)
    {
        if (mPPlaylistManager) {
            return mPPlaylistManager->getSegmentList(index);
        }
        return {};
    }

    bool playList_demuxer::is_supported(const string &uri, const uint8_t *buffer, int64_t size, int *type, const Cicada::DemuxerMeta *meta,
                                        const Cicada::options *opts)
    {
        // TODO: check the description
        int ret = HlsParser::probe(buffer, size);
        if (ret > 0) {
            *type = playList_type_hls;
            return true;
        }
        ret = Dash::MPDParser::probe(buffer, size);
        if (ret > 0) {
            *type = playList_type_dash;
            return true;
        }
        return false;
    }
    UTCTimer *playList_demuxer::getUTCTimer()
    {
        if (mPPlaylistManager) {
            return mPPlaylistManager->getUTCTimer();
        }
        return nullptr;
    }
}
