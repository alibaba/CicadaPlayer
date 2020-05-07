//
// Created by moqi on 2019-07-08.
//
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/intreadwrite.h>
};
#define LOG_TAG "avFormatDemuxer"

#include <utils/frame_work_log.h>
#include "utils/ffmpeg_utils.h"
#include <utils/errors/framework_error.h>
#include <utils/AFMediaType.h>
#include "avFormatDemuxer.h"
#include "base/media/AVAFPacket.h"
#include "AVBSF.h"
#include <mutex>
#include <utils/CicadaUtils.h>
#include <cassert>
#include <utils/timer.h>
#include <utils/CicadaJSON.h>
#include "play_list/HlsParser.h"

using namespace std;
namespace Cicada {
    static const int INITIAL_BUFFER_SIZE = 32768;

    avFormatDemuxer avFormatDemuxer::se(0);

    void avFormatDemuxer::init()
    {
        mCtx = avformat_alloc_context();
        mCtx->interrupt_callback.callback = interrupt_cb;
        mCtx->interrupt_callback.opaque = this;
        mCtx->correct_ts_overflow = 0;
        mCtx->flags |= AVFMT_FLAG_KEEP_SIDE_DATA;
#if AF_HAVE_PTHREAD
        mPthread = NEW_AF_THREAD(readLoop);
#endif
    }

    avFormatDemuxer::avFormatDemuxer()
    {
        init();
    }

    avFormatDemuxer::avFormatDemuxer(const string &path) : IDemuxer(path)
    {
        AF_TRACE;
        init();
    }

    int avFormatDemuxer::interrupt_cb(void *opaque)
    {
        return static_cast<avFormatDemuxer *>(opaque)->mInterrupted;
    }

    avFormatDemuxer::~avFormatDemuxer()
    {
        AF_TRACE;
        Close();
#if AF_HAVE_PTHREAD
        delete mPthread;
        mPthread = nullptr;
#endif
    }

    int avFormatDemuxer::Open()
    {
        return open(nullptr);
    }

    int avFormatDemuxer::open(AVInputFormat *in_fmt)
    {
        if (bOpened) {
            return 0;
        }

        int64_t startTime = af_getsteady_ms();
        bool use_filename = false;

        if (mReadCb != nullptr ) {
            uint8_t *read_buffer = static_cast<uint8_t *>(av_malloc(INITIAL_BUFFER_SIZE));
            mPInPutPb = avio_alloc_context(read_buffer, INITIAL_BUFFER_SIZE, 0, mUserArg, mReadCb, nullptr, mSeekCb);

            if (mPInPutPb == nullptr) {
                av_free(read_buffer);
                return -ENOMEM;
            }

            mCtx->pb = mPInPutPb;
        } else {
            use_filename = true;
        }

        av_dict_set_int(&mInputOpts, "safe", 0, 0);
        av_dict_set(&mInputOpts, "protocol_whitelist", "file,http,https,tcp,tls", 0);
        /*If a url with mp4 ext name, but is not a mp4 file, the mp4 demuxer will be matched
         * by ext name , mp4 demuxer will try to find moov box, it will ignore the return value
         * of the avio_*, and don't check interrupt flag, if the url is a network file, here will
         * blocking. To avoid this, when the filename is not opened by ffmpeg, change the file name
         * to a fake name ,only tell ffmpeg this is not a local file, and ffmpeg will use it
         * in:
         * ff_configure_buffers_for_index()
         * this optimally the mov demuxing  when moov box at the end of the file.
         *
         * But if the filename is used for ffmpeg to open, it's can't do this.
         */
        const char *filename = mPath.c_str();

        if (!use_filename) {
            if (CicadaUtils::startWith(mPath, {"http://", "https://"})) {
                AVInputFormat *mp4Format = av_find_input_format("mp4");

                if (mp4Format && av_match_ext(filename, mp4Format->extensions)) {
                    filename = "http://xxx";
                }
            }
        }

        //TODO: opts
        int ret = avformat_open_input(&mCtx, filename, in_fmt, mInputOpts ? &mInputOpts : nullptr);
        AVDictionaryEntry *t;

        if ((t = av_dict_get(mInputOpts, "", nullptr, AV_DICT_IGNORE_SUFFIX))) {
            AF_LOGD("Option %s not found.\n", t->key);
            // return AVERROR_OPTION_NOT_FOUND;
        }

        if (ret < 0) {
            AF_LOGE("avformat_open_input error %d,%s,", ret, getErrorString(ret));

            if (ret == AVERROR_PROTOCOL_NOT_FOUND) {
                return FRAMEWORK_ERR_PROTOCOL_NOT_SUPPORT;
            }

            if (ret == AVERROR_EXIT || mInterrupted) {
                return FRAMEWORK_ERR_EXIT;
            }

            return ret;
        }

        mCtx->flags |= AVFMT_FLAG_GENPTS;
        // TODO: add a opt to set fps probe
        mCtx->fps_probe_size = 0;
        // TODO: only find ts and flv's info?

        if (mMetaInfo) {
            for (int i = 0; i < mCtx->nb_streams; ++i) {
                if (i >= mMetaInfo->meta.size()) {
                    break;
                }

                set_stream_meta(mCtx->streams[i], (Stream_meta *) *mMetaInfo->meta[i].get());
            }

            // TODO: set it to zero can avoid read and decode more frames,but wen seek to back in hlsstream,will lead pts err
            if (mMetaInfo->bContinue) {
                mCtx->max_ts_probe = 0;
            }
        }

        ret = avformat_find_stream_info(mCtx, nullptr);

        if (mInterrupted) {
            AF_LOGD("interrupted\n");
            return FRAMEWORK_ERR_EXIT;
        }

        if (ret < 0) {
            AF_LOGE("avformat_find_stream_info error %d:%s\n", ret, getErrorString(ret));
            return ret;
        }

        /*
         * this flag is only affect on mp3 and flac
         */
        if (mCtx->duration > 600 * AV_TIME_BASE) {
            mCtx->flags |= AVFMT_FLAG_FAST_SEEK;
        }

        bOpened = true;
        int64_t used = af_getsteady_ms() - startTime;
        CicadaJSONItem json;
        json.addValue("cost", (int) used);
        json.addValue("time", (double) af_getsteady_ms());
        mProbeString = json.printJSON();

        if (mStartTime > 0 && mStartTime < mCtx->duration) {
            Seek(mStartTime, 0, -1);
        }

        return 0;
    }

    void avFormatDemuxer::Close()
    {
        Stop();

        if (mCtx) {
            avformat_close_input(&mCtx);
        }

        if (mPInPutPb) {
            av_free(mPInPutPb->buffer);
            av_free(mPInPutPb);
            mPInPutPb = nullptr;
        }

        mStreamCtxMap.clear();
        mPacketQueue.clear();
        bOpened = false;

        if (mInputOpts) {
            av_dict_free(&mInputOpts);
        }
    }

    int avFormatDemuxer::ReadPacketInternal(unique_ptr<IAFPacket> &packet)
    {
        if (!bOpened) {
            return -EINVAL;
        }

        AVPacket *pkt = av_packet_alloc();
        int err;
        av_init_packet(pkt);

        do {
            err = av_read_frame(mCtx, pkt);

            if (err < 0) {
                if (err != AVERROR(EAGAIN)) {
                    if (mCtx->pb && mCtx->pb->error != AVERROR_EXIT) {
                        av_log(NULL, AV_LOG_WARNING, "%s:%d: %s, ctx->pb->error=%d\n", __FILE__, __LINE__, getErrorString(err),
                               mCtx->pb->error);
                    }
                }

                if (mCtx->pb && mCtx->pb->error == FRAMEWORK_ERR_EXIT) {
                    av_packet_free(&pkt);
                    return FRAMEWORK_ERR_EXIT;
                }

                if (err == AVERROR_EOF) {
                    if (mCtx->pb && mCtx->pb->error == AVERROR(EAGAIN)) {
                        av_packet_free(&pkt);
                        return mCtx->pb->error;
                    }

                    if (mCtx->pb && mCtx->pb->error < 0) {
                        av_packet_free(&pkt);
                        int ret = mCtx->pb->error;
                        mCtx->pb->error = 0;
                        return ret;
                    }

                    av_packet_free(&pkt);
                    return 0;// EOS
                }

                if (err == AVERROR_EXIT) {
                    AF_LOGE("AVERROR_EXIT\n");
                    av_packet_free(&pkt);
                    return -EAGAIN;
                }

                if (err == AVERROR(EAGAIN) || err == SEGEND) {
                    if (mCtx->pb) {
                        mCtx->pb->eof_reached = 0;
                        mCtx->pb->error = 0;
                    }
                }

                av_packet_free(&pkt);
                return err;
            }

            if (mStreamCtxMap[pkt->stream_index] && mStreamCtxMap[pkt->stream_index]->opened) {
                break;
            }

            av_packet_unref(pkt);
        } while (true);

        if (pkt->pts == AV_NOPTS_VALUE) {
            AF_LOGW("pkt pts error\n");
        }

        if (pkt->dts == AV_NOPTS_VALUE) {
            AF_LOGW("pkt dts error\n");
        }

        bool needUpdateExtraData = false;
        int new_extradata_size;
        const uint8_t *new_extradata = av_packet_get_side_data(pkt,
                                       AV_PKT_DATA_NEW_EXTRADATA,
                                       &new_extradata_size);

        if (new_extradata) {
            AF_LOGI("AV_PKT_DATA_NEW_EXTRADATA");
            int streamIndex = pkt->stream_index;
            AVCodecParameters *codecpar = mCtx->streams[streamIndex]->codecpar;
            av_free(codecpar->extradata);
            codecpar->extradata = static_cast<uint8_t *>(av_malloc(new_extradata_size + AV_INPUT_BUFFER_PADDING_SIZE));
            memcpy(codecpar->extradata, new_extradata, new_extradata_size);
            codecpar->extradata_size = new_extradata_size;

            if (mStreamCtxMap[streamIndex]->bsf) {
                createBsf(streamIndex);
            } else {
                needUpdateExtraData = true;
            }
        }

        if (mStreamCtxMap[pkt->stream_index]->bsf) {
            // TODO: while pulling and ret value
            /*
             * pkt.stream_index will be reset to zero after push
             */
            // TODO: deal with error
            int index = pkt->stream_index;
            mStreamCtxMap[index]->bsf->push(pkt);
            int ret = mStreamCtxMap[index]->bsf->pull(pkt);

            if (ret < 0) {
                av_packet_free(&pkt);
                return -EAGAIN;
            }
        }

        err = pkt->size;

        if (pkt->pts != AV_NOPTS_VALUE) {
            pkt->pts = av_rescale_q(pkt->pts, mCtx->streams[pkt->stream_index]->time_base, AV_TIME_BASE_Q);
        }

        if (pkt->dts != AV_NOPTS_VALUE) {
            pkt->dts = av_rescale_q(pkt->dts, mCtx->streams[pkt->stream_index]->time_base, AV_TIME_BASE_Q);
        }

        if (pkt->duration > 0) {
            pkt->duration = av_rescale_q(pkt->duration, mCtx->streams[pkt->stream_index]->time_base, AV_TIME_BASE_Q);
        }

        packet = unique_ptr<IAFPacket>(new AVAFPacket(&pkt));

        if (needUpdateExtraData) {
            packet->setExtraData(new_extradata, new_extradata_size);
        }

        if (packet->getInfo().pts != INT64_MIN) {
            packet->getInfo().timePosition = packet->getInfo().pts - mCtx->start_time;
        }

        return err;
    }

    int avFormatDemuxer::OpenStream(int index)
    {
#if AF_HAVE_PTHREAD
        std::unique_lock<std::mutex> uLock(mMutex);
#endif

        if (index >= mCtx->nb_streams) {
            AF_LOGE("no such stream\n");
            return -EINVAL;
        }

        if (mStreamCtxMap[index] != nullptr) {
            mStreamCtxMap[index]->opened = true;
            return 0;
        }

        mStreamCtxMap[index] = unique_ptr<AVStreamCtx>(new AVStreamCtx());
        mStreamCtxMap[index]->opened = true;
        createBsf(index);
        return 0;
    }

    void avFormatDemuxer::CloseStream(int index)
    {
#if AF_HAVE_PTHREAD
        std::unique_lock<std::mutex> uLock(mMutex);
#endif

        if (mStreamCtxMap.find(index) == mStreamCtxMap.end()) {
            AF_LOGI("not opened\n");
            return;
        }

        mStreamCtxMap[index]->opened = false;
    }

    int avFormatDemuxer::createBsf(int index)
    {
        string bsfName{};
        int ret = 0;
        const AVCodecParameters *codecpar = mCtx->streams[index]->codecpar;

        if (mMergeVideoHeader) {
            if (codecpar->codec_id == AV_CODEC_ID_H264 && codecpar->extradata != nullptr && (codecpar->extradata[0] == 1)) {
                bsfName = "h264_mp4toannexb";
            } else if (codecpar->codec_id == AV_CODEC_ID_HEVC && codecpar->extradata_size >= 5 &&
                       AV_RB32(codecpar->extradata) != 0x0000001 &&
                       AV_RB24(codecpar->extradata) != 0x000001) {
                bsfName = "hevc_mp4toannexb";
            }

            // TODO: mpeg4 dump extra bsf
        } else {
            if (codecpar->codec_id == AV_CODEC_ID_H264 && codecpar->extradata != nullptr && (codecpar->extradata[0] != 1)) {
                bsfName = "h26xAnnexb2xVcc";
            } else if (codecpar->codec_id == AV_CODEC_ID_HEVC && codecpar->extradata_size >= 5 &&
                       !(AV_RB32(codecpar->extradata) != 0x0000001 &&
                         AV_RB24(codecpar->extradata) != 0x000001)) {
                bsfName = "h26xAnnexb2xVcc";
            }
        }

        if (!bsfName.empty()) {
            mStreamCtxMap[index]->bsf = unique_ptr<IAVBSF>(IAVBSFFactory::create(bsfName));
            ret = mStreamCtxMap[index]->bsf->init(bsfName, mCtx->streams[index]->codecpar);

            if (ret < 0) {
                AF_LOGE("create %s bsf error\n", bsfName.c_str());
                mStreamCtxMap[index] = nullptr;
            }
        }

        return ret;
    }

    void avFormatDemuxer::flush()
    {
        avio_flush(mCtx->pb);
        avformat_flush(mCtx);
        mError = 0;
    }

    void avFormatDemuxer::interrupt(int inter)
    {
        mInterrupted = inter;
    }

    int avFormatDemuxer::Seek(int64_t us, int flags, int index)
    {
        if (!bOpened) {
            mStartTime = us;
            return static_cast<int>(us);
        }

        if (mInterruptCb) {
            mInterruptCb(mUserArg, 1);
        }

        bPaused = true;
#if AF_HAVE_PTHREAD
        {
            std::unique_lock<std::mutex> waitLock(mQueLock);
            bPaused = true;
        }
        mQueCond.notify_one();
        mPthread->pause();
#else
        bPaused = true;
#endif

        if (mInterruptCb) {
            mInterruptCb(mUserArg, 0);
        }
        if(mCtx->pb->error < 0) {
            mCtx->pb->error = 0;
            avio_feof(mCtx->pb);
        }

        mPacketQueue.clear();
        mError = 0;
        static const int jitter = 2;
        int64_t timestamp = mCtx->start_time + us;
        int64_t timestamp_seek;
        int ret;

        if (!flags) { //prev
            timestamp_seek = timestamp + AV_TIME_BASE * jitter;
            ret = avformat_seek_file(mCtx, -1, INT64_MIN, timestamp, timestamp_seek, 0);
        } else {
            if (timestamp - AV_TIME_BASE * jitter > INT64_MIN) {
                timestamp_seek = timestamp - AV_TIME_BASE * jitter;
            } else {
                timestamp_seek = INT64_MIN;
            }

            ret = avformat_seek_file(mCtx, -1, timestamp_seek, timestamp, INT64_MAX, 0);
        }

        bEOS = false;
        bPaused = false;
#if AF_HAVE_PTHREAD

        if (mPthread->getStatus() == afThread::THREAD_STATUS_PAUSED) {
            mPthread->start();
        }

#endif
        return ret;
    }

    int avFormatDemuxer::GetNbStreams()
    {
        return mCtx->nb_streams;
    }


    int avFormatDemuxer::GetSourceMeta(Source_meta **meta)
    {
        return AVDictionary2SourceMeta(meta, mCtx->metadata);
    }

    int avFormatDemuxer::GetStreamMeta(Stream_meta *meta, int index, bool sub)
    {
        if (index < 0 || index > mCtx->nb_streams) {
            return -EINVAL;
        }

        get_stream_meta(mCtx->streams[index], meta);

        if (mCtx->duration != AV_NOPTS_VALUE) {
            meta->duration = mCtx->duration;
        } else {
            meta->duration = 0;
        }

        meta->index = index;
        return 0;
    }

    void avFormatDemuxer::Start()
    {
#if AF_HAVE_PTHREAD
        bPaused = false;
        mPthread->start();
#endif
    }

    void avFormatDemuxer::Stop()
    {
#if AF_HAVE_PTHREAD
        {
            std::unique_lock<std::mutex> waitLock(mQueLock);
            bPaused = true;
        }
        mQueCond.notify_one();

        if (mPthread) {
            mPthread->stop();
        }

#endif
    }

#if AF_HAVE_PTHREAD

    int avFormatDemuxer::readLoop()
    {
        if (bPaused) {
            return 0;
        }

        if (bEOS) {
            std::unique_lock<std::mutex> waitLock(mQueLock);

            if (bEOS) {
                mQueCond.wait(waitLock, [this]() {
                    return bPaused || mInterrupted;
                });
            }
        }

        if (bEOS || bPaused) {
            return 0;
        }

        unique_ptr<IAFPacket> pkt{};
        int ret = ReadPacketInternal(pkt);

        if (ret > 0) {
            std::unique_lock<std::mutex> waitLock(mQueLock);

            if (mPacketQueue.size() > MAX_QUEUE_SIZE) {
                mQueCond.wait(waitLock, [this]() {
                    return mPacketQueue.size() <= MAX_QUEUE_SIZE || bPaused || mInterrupted;
                });
            }

            mPacketQueue.push_back(std::move(pkt));
        } else if (ret == 0) {
            bEOS = true;
        } else {
            if (ret != AVERROR(EAGAIN) && ret != FRAMEWORK_ERR_EXIT) {
                mError = ret;
            }

            std::unique_lock<std::mutex> waitLock(mQueLock);
            mQueCond.wait_for(waitLock, std::chrono::milliseconds(10), [this]() {
                return bPaused || mInterrupted;
            });
        }

        return 0;
    }

#endif

    int avFormatDemuxer::ReadPacket(std::unique_ptr<IAFPacket> &packet, int index)
    {
#if !AF_HAVE_PTHREAD
        return ReadPacketInternal(packet);
#else

        if (mPthread->getStatus() == afThread::THREAD_STATUS_IDLE) {
            return ReadPacketInternal(packet);
        } else {
            std::unique_lock<std::mutex> waitLock(mQueLock);

            if (!mPacketQueue.empty()) {
                packet = std::move(mPacketQueue.front());
                mPacketQueue.pop_front();
                mQueCond.notify_one();
                return static_cast<int>(packet->getSize());
            }

            if (bEOS) {
                return 0;
            }

            if (mError < 0) {
                return mError;
            }

            return -EAGAIN;
        }

#endif
    }

    const std::string avFormatDemuxer::GetProperty(int index, const string &key)
    {
        if (key == "probeInfo") {
            return mProbeString;
        }

        return "";
    }

    bool avFormatDemuxer::is_supported(const string &uri, const uint8_t *buffer, int64_t size, int *type, const Cicada::DemuxerMeta *meta,
                                       const Cicada::options *opts)
    {
#ifdef ENABLE_HLS_DEMUXER

        if (HlsParser::probe(buffer, size) > 0) {
            return false;
        }

#endif
        AVProbeData pd = {uri.c_str(), const_cast<unsigned char *>(buffer), static_cast<int>(size)};
        int score = AVPROBE_SCORE_RETRY;
        AVInputFormat *fmt = av_probe_input_format2(&pd, 1, &score);

        if (fmt &&
                (strcmp(fmt->name, "hls,applehttp") == 0
                 || strcmp(fmt->name, "webvtt") == 0)) {
            return false;
        }

        // TODO: use more and more data to probe
        if (fmt || 1) {
            *type = demuxer_type_bit_stream;
            return true;
        }

        return false;
    }

    avFormatDemuxer::avFormatDemuxer(int dummy)
    {
        addPrototype(this);
        ffmpeg_init();
    }

    void avFormatDemuxer::PreStop()
    {
#if AF_HAVE_PTHREAD
        std::unique_lock<std::mutex> waitLock(mQueLock);
        bPaused = true;
        mQueCond.notify_one();
#endif
    }

}
