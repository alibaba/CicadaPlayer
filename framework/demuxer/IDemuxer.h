//
// Created by moqi on 2018/2/1.
//

#ifndef FRAMEWORK_DEMUXER_H
#define FRAMEWORK_DEMUXER_H

#include "base/media/framework_type.h"
#include "utils/AFMediaType.h"
#include "play_list/playList.h"

#include <functional>
#include <string>
#include <utility>
#include <utils/CicadaType.h>
#include <base/media/IAFPacket.h>
#include <utils/mediaTypeInternal.h>
#include <base/OptionOwner.h>
#include <data_source/IDataSource.h>
#include "DemuxerMetaInfo.h"

namespace Cicada {
    typedef enum demuxer_type {
        demuxer_type_unknown = 0,
        demuxer_type_playlist,
        demuxer_type_bit_stream,
        demuxer_type_webvtt,
    } demuxer_type;


    class CICADA_CPLUS_EXTERN IDemuxer : public OptionOwner {
    public:


        IDemuxer();

        explicit IDemuxer(string path);

        virtual void setDataSourceConfig(const IDataSource::SourceConfig &config)
        {
            sourceConfig = config;
        };

        virtual ~IDemuxer();

        void SetDataCallBack(demuxer_callback_read read, demuxer_callback_seek seek, demuxer_callback_open open,
                             demuxer_callback_interrupt_data inter, void *arg);

        void setMeta(DemuxerMetaInfo *metaInfo)
        {
            mMetaInfo = metaInfo;
        }

        virtual int Open() = 0;


        /**
         *
         * @param packet
         * @param index
         * @return
         *          FRAMEWORK_ERROR_NEW_FORMAT: new header data of the packet index
         */

        virtual int ReadPacket(std::unique_ptr<IAFPacket> &packet, int index) = 0;

        virtual void Close() = 0;

        virtual void Start() = 0;

        virtual void Stop() = 0;

        /**
         * for some udp demuxer to reconnect to server when network changed
         */
        virtual void Reload()
        {}

        virtual void PreStop(){};

        virtual void flush() = 0;

        virtual int Seek(int64_t us, int flags, int index) = 0;

        virtual int GetNbStreams() = 0;

        virtual int GetNbSubStreams(int index)
        {
            return -1;
        };

        virtual int GetRemainSegmentCount(int index)
        {
            return -1;
        }

        virtual int GetSourceMeta(Source_meta **meta) = 0;

        attribute_deprecated
        virtual int GetStreamMeta(Stream_meta *meta, int index, bool sub) = 0;

        virtual int GetStreamMeta(unique_ptr<streamMeta> &meta, int index, bool sub);

        virtual int OpenStream(int index) = 0;

        virtual void CloseStream(int index) = 0;

        virtual int SwitchStreamAligned(int from, int to)
        {
            return -1;
        }


        virtual void interrupt(int inter) = 0;

        virtual const playList *GetPlayList()
        {
            return nullptr;
        }

        virtual bool isPlayList()
        {
            return false;
        }

        virtual void setDataSourceIO()
        {

        }

        virtual void *getCodecPar(int streamIndex)
        {
            return nullptr;
        }

        virtual const std::string GetProperty(int index, const string &key)
        { return ""; }

        virtual void SetOption(const options *opts)
        {
            mOpts = opts;
        };

        virtual int SetOption(const std::string &key, const int64_t value)
        {
            return 0;
        }

        virtual void setBitStreamFormat(bool vMergeHeader, bool aMergeHeader)
        {
            mMergeVideoHeader = vMergeHeader;
            mMergerAudioHeader = aMergeHeader;
        }

        virtual void setDemuxerCb(std::function<void(std::string, std::string)> func)
        { mDemuxerCbfunc = func; }

    protected:
        demuxer_callback_read mReadCb{nullptr};
        demuxer_callback_seek mSeekCb{nullptr};
        demuxer_callback_open mOpenCb{nullptr};
        demuxer_callback_interrupt_data mInterruptCb{nullptr};
        void *mUserArg{nullptr};
        std::function<void(std::string, std::string)> mDemuxerCbfunc;
        string mPath{};
        IDataSource::SourceConfig sourceConfig{};

        bool mMergeVideoHeader = false;
        bool mMergerAudioHeader = false;

        DemuxerMetaInfo *mMetaInfo = nullptr;
    };
}

#endif //FRAMEWORK_DEMUXER_H
