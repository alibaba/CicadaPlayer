//
// Created by moqi on 2019-07-08.
//

#ifndef FRAMEWORK_AVFORMATDEMUXER_H
#define FRAMEWORK_AVFORMATDEMUXER_H

#include <string>
#include <map>
#include <mutex>
#include <atomic>
#include <deque>
#include "base/media/IAFPacket.h"
#include "AVBSF.h"
#include <demuxer/IDemuxer.h>
#include <af_config.h>
#include "demuxerPrototype.h"

#if AF_HAVE_PTHREAD

#include <utils/afThread.h>

#endif

#include <utils/CicadaType.h>

struct AVFormatContext;
struct AVIOContext;
struct AVDictionary;
typedef struct AVInputFormat AVInputFormat;
namespace Cicada {

    class CICADA_CPLUS_EXTERN avFormatDemuxer : public IDemuxer, protected demuxerPrototype {

    private:
        class AVStreamCtx {
        public:
            std::unique_ptr<IAVBSF> bsf{};
            bool opened = true;
        };

    public:
        avFormatDemuxer();

        explicit avFormatDemuxer(const string &path);

        ~avFormatDemuxer() override;

        virtual int Open() override;

        int GetNbStreams() override;

        int GetSourceMeta(Source_meta **meta) override;

        int GetStreamMeta(Stream_meta *meta, int index, bool sub) override;

        void Start() override;

        void PreStop() override;

        void Stop() override;

        void flush() override;

        void interrupt(int inter) override;

        void Close() final;

        int OpenStream(int index) override;

        void CloseStream(int index) override;

        int Seek(int64_t us, int flags, int index) override;

        int ReadPacket(std::unique_ptr<IAFPacket> &packet, int index) override;

        virtual const std::string GetProperty(int index, const string &key) override;

    protected:
        explicit avFormatDemuxer(int dummy);

    private:

        Cicada::IDemuxer *clone(const string &uri, int type, const Cicada::DemuxerMeta *meta) override
        {
            return new avFormatDemuxer(uri);
        }

        bool is_supported(const string &uri, const uint8_t *buffer, int64_t size, int *type, const Cicada::DemuxerMeta *meta,
                          const Cicada::options *opts) override;

        int getType() override
        {
            return demuxer_type_bit_stream;
        }

        static avFormatDemuxer se;

    protected:
        int open(AVInputFormat *in_fmt);

    private:
        static int interrupt_cb(void *opaque);

        void init();

        int createBsf(int index);

        int ReadPacketInternal(std::unique_ptr<IAFPacket> &packet);

#if AF_HAVE_PTHREAD

        int readLoop();

#endif


    protected:
        AVDictionary *mInputOpts = nullptr;
        std::string mProbeString{};
        AVFormatContext *mCtx = nullptr;
        int MAX_QUEUE_SIZE = 60; // about 500ms  video and audio packet

    private:
        std::atomic_bool mInterrupted{false};
        std::map<int, std::unique_ptr<AVStreamCtx>> mStreamCtxMap{};
        AVIOContext *mPInPutPb = nullptr;
        bool bOpened{false};
        int64_t mStartTime = INT64_MIN;
        std::deque<unique_ptr<IAFPacket>> mPacketQueue{};
        std::atomic_bool bEOS{false};
        std::atomic_bool bPaused{false};
#if AF_HAVE_PTHREAD
        afThread *mPthread{nullptr};
        std::mutex mMutex{};
        std::mutex mQueLock{};
        std::condition_variable mQueCond{};
        atomic <int64_t> mError{0};
#endif

    };
}

#endif //FRAMEWORK_AVFORMATDEMUXER_H
