//
// Created by moqi on 2019/10/30.
//

#ifndef CICADA_PLAYER_AVFORMATSUBTITLEDEMUXER_H
#define CICADA_PLAYER_AVFORMATSUBTITLEDEMUXER_H

#include "IDemuxer.h"
#include "demuxerPrototype.h"
#include "utils/ffmpeg_utils.h"
#include <map>

extern "C" {
#include <libavformat/avformat.h>
};

namespace Cicada {
    class avFormatSubtitleDemuxer : public IDemuxer, private demuxerPrototype {

        static const int INITIAL_BUFFER_SIZE = 32768;
    public:
        avFormatSubtitleDemuxer();

        ~avFormatSubtitleDemuxer() override;

        int Open() override;

        int GetNbStreams() const override;

        int GetSourceMeta(Source_meta **meta) const override;

        int GetStreamMeta(Stream_meta *meta, int index, bool sub) const override;

        void Start() override;

        void Stop() override;

        void flush() override;

        void interrupt(int inter) override;

        void Close() override;

        int OpenStream(int index) override;

        void CloseStream(int index) override;

        int64_t Seek(int64_t us, int flags, int index) override;

        int ReadPacket(std::unique_ptr<IAFPacket> &packet, int index) override;


        explicit avFormatSubtitleDemuxer(int dummy)
        {
            ffmpeg_init();
            addPrototype(this);
        };
    private:

        Cicada::IDemuxer *clone(const string &uri, int type, const Cicada::DemuxerMeta *meta) override
        {
            return new avFormatSubtitleDemuxer();
        }

        bool is_supported(const string &uri, const uint8_t *buffer, int64_t size, int *type, const Cicada::DemuxerMeta *meta,
                          const Cicada::options *opts) override;

        int getType() override
        {
            return demuxer_type_bit_stream;
        }

        static avFormatSubtitleDemuxer se;

    private:
        static int interrupt_cb(void *opaque);

        int readPacketInternal();

    private:
        using map  = std::map<int64_t, std::unique_ptr<IAFPacket>>;
        bool bOpened{false};
        AVFormatContext *mCtx = nullptr;
        AVIOContext *mPInPutPb = nullptr;
        bool mInterrupted{false};
        map mPacketMap;
        map::iterator mCurrent = mPacketMap.end();
        int64_t mCurrentPts = INT64_MIN;
        int64_t mSeekPTS = INT64_MIN;
    };
}


#endif //CICADA_PLAYER_AVFORMATSUBTITLEDEMUXER_H
