//
// Created by moqi on 2018/2/1.
//

#ifndef FRAMEWORK_DEMUXER_H
#define FRAMEWORK_DEMUXER_H

#include "base/media/framework_type.h"
#include "play_list/playList.h"
#include "utils/AFMediaType.h"

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

using namespace std;

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

    typedef enum client_buffer_level {
        client_buffer_level_unknown,
        client_buffer_level_low,
        client_buffer_level_normal,
        client_buffer_level_low_full,
    } client_buffer_level;

    typedef void (*demuxer_callback_setSegmentList)(void *arg, const std::vector<Cicada::mediaSegmentListEntry> &segments);
    typedef int64_t (*demuxer_callback_getBufferDuration)(void *arg, int index);
    typedef void (*demuxer_callback_enableCache)(void *arg, const std::string &originUrl, bool enable);

    class CICADA_CPLUS_EXTERN IDemuxer : public OptionOwner, public IDCA, public IMediaInfoProvider {
    public:
        class streamIndexEntryInfo {
        public:
            struct entryInfo {
                int64_t mPos;
                int64_t mTimestamp;
                bool mKey;
                bool mDiscard;
                int32_t mSize;
                entryInfo(int64_t pos, int64_t timestamp, bool key, bool discard, int32_t size)
                    : mPos(pos), mTimestamp(timestamp), mKey(key), mDiscard(discard), mSize(size)
                {}
            };

        public:
            streamIndexEntryInfo() = default;
            ~streamIndexEntryInfo() = default;

        public:
            int64_t mDuration{0};
            Stream_type type{STREAM_TYPE_UNKNOWN};
            std::vector<entryInfo> mEntry;
        };

    public:
        IDemuxer();

        explicit IDemuxer(string path);

        virtual void setDataSourceConfig(const IDataSource::SourceConfig &config)
        {
            sourceConfig = config;
        };

        ~IDemuxer() override;

        void SetDataCallBack(demuxer_callback_read read, demuxer_callback_seek seek, demuxer_callback_open open,
                             demuxer_callback_interrupt_data inter, demuxer_callback_setSegmentList setSegmentList,
                             demuxer_callback_getBufferDuration getBufferDuration, demuxer_callback_enableCache enableCache, void *arg);

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

        attribute_deprecated virtual int GetStreamMeta(Stream_meta *meta, int index, bool sub) const = 0;

        virtual int GetStreamMeta(unique_ptr<streamMeta> &meta, int index, bool sub) const;

        virtual int GetMediaMeta(Media_meta *mediaMeta) const = 0;

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
        {}

        virtual void *getCodecPar(int streamIndex)
        {
            return nullptr;
        }

        virtual const std::string GetProperty(int index, const string &key) const
        {
            return "";
        }

        virtual int SetOption(const std::string &key, const int64_t value)
        {
            return 0;
        }

        virtual int SetOption(const std::string &key, const std::string &value)
        {
            return 0;
        }

        virtual void setBitStreamFormat(header_type vMergeHeader, header_type aMergeHeader)
        {
            mMergeVideoHeader = vMergeHeader;
            mMergeAudioHeader = aMergeHeader;
        }

        virtual void setDemuxerCb(std::function<void(std::string, std::string)> func)
        {
            mDemuxerCbfunc = func;
        }

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

        virtual bool isWallclockTimeSyncStream(int index)
        {
            return false;
        }

        virtual int64_t getDurationToStartStream(int index)
        {
            return 0;
        }
        virtual bool isTSDiscontinue() = 0;

        virtual const vector<streamIndexEntryInfo> &getStreamIndexEntryInfo()
        {
            return mEntryInfos;
        }

        virtual void setUrlToUniqueIdCallback(UrlHashCB cb, void *userData)
        {}

        virtual UTCTimer *getUTCTimer()
        {
            return nullptr;
        }

        virtual void setClientBufferLevel(client_buffer_level level)
        {}

    public:
        int64_t estimateExclusiveEndPositionBytes(const string &url, int64_t timeMicSec, int64_t totalLength) override;

        int64_t estimatePlayTimeMicSec(const string &url, int64_t filePosition, int64_t totalLength) override;

        std::pair<int64_t, int64_t> estimatePlayTimeMicSecRange(const pair<int64_t, int64_t> &fileRange) override;

        bool isStreamsEncodedSeparately(const string &url) override;

    protected:
        demuxer_callback_read mReadCb{nullptr};
        demuxer_callback_seek mSeekCb{nullptr};
        demuxer_callback_open mOpenCb{nullptr};
        demuxer_callback_interrupt_data mInterruptCb{nullptr};
        demuxer_callback_setSegmentList mSetSegmentList{nullptr};
        demuxer_callback_getBufferDuration mGetBufferDuration{nullptr};
        demuxer_callback_enableCache mEnableCache{nullptr};
        void *mUserArg{nullptr};
        std::function<void(std::string, std::string)> mDemuxerCbfunc;
        string mPath{};
        IDataSource::SourceConfig sourceConfig{};

        header_type mMergeVideoHeader = header_type ::header_type_no_touch;
        header_type mMergeAudioHeader = header_type ::header_type_no_touch;

        DemuxerMetaInfo *mMetaInfo = nullptr;
        std::string mName = "IDemuxer";
        std::vector<streamIndexEntryInfo> mEntryInfos;
    };
}// namespace Cicada

#endif//FRAMEWORK_DEMUXER_H
