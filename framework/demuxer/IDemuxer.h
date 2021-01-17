//
// Created by moqi on 2018/2/1.
//

#ifndef FRAMEWORK_DEMUXER_H
#define FRAMEWORK_DEMUXER_H

#include "base/media/framework_type.h"
#include "utils/AFMediaType.h"
#include "play_list/playList.h"

#include "DemuxerMetaInfo.h"
#include <base/IDCA.h>
#include <base/OptionOwner.h>
#include <base/media/IAFPacket.h>
#include <data_source/IDataSource.h>
#include <functional>
#include <string>
#include <utility>
#include <utils/CicadaType.h>
#include <utils/mediaTypeInternal.h>

namespace Cicada {
    typedef enum demuxer_type {
        demuxer_type_unknown = 0,
        demuxer_type_playlist,
        demuxer_type_bit_stream,
        demuxer_type_webvtt,
    } demuxer_type;

    typedef enum header_type {
        header_type_no_touch,
        header_type_merge,
        header_type_extract,
    } header_type;

    class CICADA_CPLUS_EXTERN IDemuxer : public OptionOwner, public IDCA {
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

        virtual int64_t Seek(int64_t us, int flags, int index) = 0;

        virtual int GetNbStreams() const = 0;

        virtual int GetNbSubStreams(int index) const
        {
            return -1;
        };

        virtual int GetRemainSegmentCount(int index)
        {
            return -1;
        }

        virtual int GetSourceMeta(Source_meta **meta) const = 0;

        attribute_deprecated
        virtual int GetStreamMeta(Stream_meta *meta, int index, bool sub) const = 0;

        virtual int GetStreamMeta(unique_ptr<streamMeta> &meta, int index, bool sub) const;

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

        virtual bool isPlayList() const
        {
            return false;
        }

        virtual int64_t getMaxGopTimeUs()
        {
            return INT64_MIN;
        }

        virtual void setDataSourceIO()
        {

        }

        virtual void *getCodecPar(int streamIndex)
        {
            return nullptr;
        }

        virtual const std::string GetProperty(int index, const string &key) const
        { return ""; }

        virtual int SetOption(const std::string &key, const int64_t value)
        {
            return 0;
        }

        virtual int SetOption(const std::string &key, const std::string& value)
        {
            return 0;
        }

        virtual void setBitStreamFormat(header_type vMergeHeader, header_type aMergeHeader)
        {
            mMergeVideoHeader = vMergeHeader;
            mMergeAudioHeader = aMergeHeader;
        }

        virtual void setDemuxerCb(std::function<void(std::string, std::string)> func)
        { mDemuxerCbfunc = func; }

        virtual int64_t getBufferDuration(int index) const
        {
            return 0;
        }

        virtual bool isLowLatency() const
        {
            return false;
        }

        int invoke(int cmd, const std::string &content) override
        {
            return 0;
        }

        virtual std::string getName() const
        {
            return mName;
        }
        
        virtual bool isRealTimeStream(int index)
        {
            return false;
        }

    protected:
        demuxer_callback_read mReadCb{nullptr};
        demuxer_callback_seek mSeekCb{nullptr};
        demuxer_callback_open mOpenCb{nullptr};
        demuxer_callback_interrupt_data mInterruptCb{nullptr};
        void *mUserArg{nullptr};
        std::function<void(std::string, std::string)> mDemuxerCbfunc;
        string mPath{};
        IDataSource::SourceConfig sourceConfig{};

        header_type mMergeVideoHeader = header_type ::header_type_no_touch;
        header_type mMergeAudioHeader = header_type ::header_type_no_touch;

        DemuxerMetaInfo *mMetaInfo = nullptr;
        std::string mName = "IDemuxer";
    };
}

#endif //FRAMEWORK_DEMUXER_H
