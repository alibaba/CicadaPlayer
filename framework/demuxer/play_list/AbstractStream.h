//
// Created by moqi on 2018/4/27.
//

#ifndef FRAMEWORK_ABSTRACTSTREAM_H
#define FRAMEWORK_ABSTRACTSTREAM_H

/*
 * AbstractStream is a single stream proved form playList file,
 * maybe a ts muxed with one or more es stream,or a aac file
 */
#include <demuxer/demuxer_service.h>
#include "utils/AFMediaType.h"
#include <base/OptionOwner.h>

namespace Cicada{
    class AbstractStream : public OptionOwner {
    public:
        AbstractStream();

        virtual ~AbstractStream();

        virtual int open() = 0;

        virtual void close() = 0;

        virtual int read(std::unique_ptr<IAFPacket> &packet ) = 0;

        virtual int GetNbStreams() = 0;

        virtual int GetStreamMeta(Stream_meta *meta, int index, bool sub) = 0;

        virtual bool isOpened() = 0;

        virtual int start() = 0;

        virtual int stop() = 0;

        virtual int64_t seek(int64_t us, int flags) = 0;

        virtual uint64_t getCurSegNum() = 0;

        virtual int stopOnSegEnd(bool stop) = 0;

        virtual int SetCurSegNum(uint64_t num) = 0;

        virtual uint64_t getCurSegPosition() = 0;

        virtual int setCurSegPosition(uint64_t position) = 0;

        virtual bool isLive() = 0;

        virtual int64_t getDuration() = 0;

        virtual int getNBStream() = 0;

        virtual void interrupt(int inter) = 0;

        virtual void setExtDataSource(IDataSource *source)
        {
            mExtDataSource = source;
        }


        virtual void setDataSourceConfig(const IDataSource::SourceConfig &config)
        {
            mSourceConfig = config;
        }

        virtual void setBitStreamFormat(bool vMergeHeader, bool aMergeHeader)
        {
            mMergeVideoHeader = vMergeHeader;
            mMergerAudioHeader = aMergeHeader;
        }

    protected:
        IDataSource *mExtDataSource = nullptr;
        IDataSource::SourceConfig mSourceConfig{};
        bool mMergeVideoHeader = false;
        bool mMergerAudioHeader = false;
    };
}


#endif //FRAMEWORK_ABSTRACTSTREAM_H
