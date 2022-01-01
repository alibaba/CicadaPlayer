//
// Created by moqi on 2018/4/27.
//

#ifndef FRAMEWORK_PLAYLISTMANAGER_H
#define FRAMEWORK_PLAYLISTMANAGER_H

#include <demuxer/demuxer_service.h>
#include "playList.h"
#include "utils/AFMediaType.h"
#include <memory>
#include <base/OptionOwner.h>

namespace Cicada{
    class PlaylistManager : public OptionOwner {
    public:
        explicit PlaylistManager(playList *pList);

        virtual ~PlaylistManager();

        virtual int init() = 0;

        virtual int start() = 0;

        virtual void stop() = 0;

        virtual int GetNbStreams() const = 0;

        virtual int GetStreamMeta(Stream_meta *meta, int index, bool sub) const = 0;

        virtual int ReadPacket(std::unique_ptr<IAFPacket> &, int index) = 0;

        virtual int OpenStream(int id) = 0;

        virtual void CloseStream(int id) = 0;

        virtual int64_t seek(int64_t us, int flags, int index) = 0;

        virtual int SwitchStreamAligned(int from, int to) = 0;

        virtual int getNBSubStream(int index) const = 0;

        virtual void interrupt(int inter) = 0;

        virtual const std::string GetProperty(int index, const string &key) = 0;

        virtual int GetRemainSegmentCount(int index) = 0;

        virtual void setExtDataSource(IDataSource *source)
        {
            mExtDataSource = source;
        }

        virtual void setDataSourceConfig(const IDataSource::SourceConfig &config)
        {
            mSourceConfig = config;
        }

        virtual void setBitStreamFormat(header_type vMergeHeader, header_type aMergeHeader)
        {
            mMergeVideoHeader = vMergeHeader;
            mMergerAudioHeader = aMergeHeader;
        }
        
        virtual bool isRealTimeStream(int index) = 0;

        virtual int64_t getTargetDuration() = 0;

    protected:
        playList *mPList = nullptr;
        IDataSource *mExtDataSource = nullptr;
        IDataSource::SourceConfig mSourceConfig{};
        header_type mMergeVideoHeader = header_type::header_type_no_touch;
        header_type mMergerAudioHeader = header_type::header_type_no_touch;
    };
}


#endif //FRAMEWORK_PLAYLISTMANAGER_H
