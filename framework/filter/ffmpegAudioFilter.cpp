//
// Created by moqi on 2018/10/30.
//
#define LOG_TAG "ffmpegAudioFilter"

#include <utils/timer.h>
#include <utils/frame_work_log.h>
#include <mutex>
#include <base/media/AVAFPacket.h>
#include "ffmpegAudioFilter.h"
#include "utils/AutoAVFrame.h"
#include <utils/ffmpeg_utils.h>
#include <cstdint>
#include <cinttypes>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
}

#define MAX_INPUT_BUFFER_COUNT 2
//#define DUMP_PCM
#ifdef DUMP_PCM
    static int fd;

    #include <fcntl.h>
    #include <unistd.h>

#endif

namespace Cicada {

    ffmpegAudioFilter::ffmpegAudioFilter(const format &srcFormat, const format &dstFormat) : IAudioFilter(srcFormat, dstFormat)
    {
        mRate = 1.0;
        mPThread = nullptr;
        avfilter_register_all();
#ifdef DUMP_PCM
        fd = open("out.pcm", O_CREAT | O_RDWR, 0666);
#endif
    }

    ffmpegAudioFilter::~ffmpegAudioFilter()
    {
        delete mPThread;
        mPThread = nullptr;
        avfilter_graph_free(&m_pFilterGraph);
        flush();
    }

    bool ffmpegAudioFilter::isSupported(string capacity)
    {
        if (capacity == "atempo") {
            return true;
        }

        return false;
    }

    bool ffmpegAudioFilter::setOption(const string &key, const string &value, const string &capacity)
    {
        if (capacity == "atempo") {
            if (key == "rate") {
                if (mRate != atof(value.c_str())) {
                    std::lock_guard<std::mutex> uMutex(mMutexRate);
                    mRate = atof(value.c_str());

                    if (m_pFilterGraph) {
                        avfilter_graph_send_command(m_pFilterGraph, "atempo", "tempo", value.c_str(), nullptr, 0, 0);
                    }
                }

                return true;
            }
        } else if (capacity == "volume") {
            if (mVolume != atof(value.c_str())) {
                mVolume = atof(value.c_str());

                if (m_pFilterGraph) {
                    avfilter_graph_send_command(m_pFilterGraph, "volume", "volume", value.c_str(), nullptr, 0, 0);
                }
            }

            return true;
        }

        return false;
    }

    int ffmpegAudioFilter::createSrcBufferFilter()
    {
        char ch_layout[64];
        const AVFilter *abuffer = avfilter_get_by_name("abuffer");
        mAbuffer_ctx = avfilter_graph_alloc_filter(m_pFilterGraph, abuffer, "src");

        if (mAbuffer_ctx == nullptr) {
            return -EINVAL;
        }

        AVRational r;
        r.den = 1000000;
        r.num = 1;
        uint64_t channel_layout = mSrcFormat.channel_layout;

        if (!channel_layout) {
            channel_layout = av_get_default_channel_layout(mSrcFormat.channels);
        }

        /* Set the filter options through the AVOptions API. */
        av_get_channel_layout_string(ch_layout, sizeof(ch_layout), 0, channel_layout);
        av_opt_set(mAbuffer_ctx, "channel_layout", ch_layout, AV_OPT_SEARCH_CHILDREN);
        av_opt_set(mAbuffer_ctx, "sample_fmt", av_get_sample_fmt_name((enum AVSampleFormat) mSrcFormat.format), AV_OPT_SEARCH_CHILDREN);
        av_opt_set_q(mAbuffer_ctx, "time_base", r, AV_OPT_SEARCH_CHILDREN);
        av_opt_set_int(mAbuffer_ctx, "sample_rate", mSrcFormat.sample_rate, AV_OPT_SEARCH_CHILDREN);
        /* Now initialize the filter; we pass NULL options, since we have already
        * set all the options above. */
        int err = avfilter_init_str(mAbuffer_ctx, nullptr);

        if (err < 0) {
            AF_LOGE("Could not initialize the abuffer filter.\n");
        }

        return err;
    }

    AVFilterContext *ffmpegAudioFilter::createFilter(const char *name, const char *opt)
    {
        const AVFilter *filter = avfilter_get_by_name(name);
        AVFilterContext *filterContext = avfilter_graph_alloc_filter(m_pFilterGraph, filter, name);

        if (filterContext == nullptr) {
            return nullptr;
        }

        int err = avfilter_init_str(filterContext, opt);

        if (err < 0) {
            AF_LOGE("Could not initialize the %s filter.\n", opt);
            avfilter_free(filterContext);
            return nullptr;
        }

        return filterContext;
    }

    int ffmpegAudioFilter::addFilter(AVFilterContext **current, const char *name, const char *options_str)
    {
        int err;
        AVFilterContext *filterContext = createFilter(name, options_str);

        if (filterContext == nullptr) {
            AF_LOGW("%s filter create fail\n", name);
            return -EINVAL;
        } else {
            err = avfilter_link(*current, 0, filterContext, 0);

            if (err < 0) {
                AF_LOGW("%s filter link fail\n", name);
                return -EINVAL;
            }

            *current = filterContext;
        }

        return err;
    }

    int ffmpegAudioFilter::init()
    {
        int err;
        m_pFilterGraph = avfilter_graph_alloc();

        if (m_pFilterGraph == nullptr) {
            return -ENOMEM;
        }

        AVFilterContext *current = nullptr;
        err = createSrcBufferFilter();

        if (err < 0) {
            return err;
        }

        mAbuffersink_ctx = createFilter("abuffersink", nullptr);

        if (mAbuffersink_ctx == nullptr) {
            return -EINVAL;
        }

        current = mAbuffer_ctx;
        char options_str[1024];
        bool needAFormat = false;
        // for now enlarge only
        snprintf(options_str, sizeof(options_str), "volume=%f", std::max(mVolume, 1.0));
        err = addFilter(&current, "volume", options_str);

        if (err == 0 //volume would add a aformat filter too
                || mSrcFormat.sample_rate != mDstFormat.sample_rate
                || mSrcFormat.format != mDstFormat.format
                || mSrcFormat.channels != mDstFormat.channels) {
            needAFormat = true;
        }

        snprintf(options_str, sizeof(options_str), "tempo=%f", mRate.load());
        addFilter(&current, "atempo", options_str);

        if (needAFormat) {
            snprintf(options_str, sizeof(options_str),
                     "sample_fmts=%s:sample_rates=%d:channel_layouts=0x%" PRIx64,
                     av_get_sample_fmt_name((enum AVSampleFormat) mDstFormat.format),
                     mDstFormat.sample_rate,
                     av_get_default_channel_layout(mDstFormat.channels));
            addFilter(&current, "aformat", options_str);
        }

        err = avfilter_link(current, 0, mAbuffersink_ctx, 0);
        err = avfilter_graph_config(m_pFilterGraph, nullptr);
        mFirstPts = INT64_MIN;
        mDeltaPts = 0;
        mLastInputPts = INT64_MIN;
        mLastInPutDuration = 0;

        if (mPThread == nullptr) {
            mPThread = NEW_AF_THREAD(FilterLoop);
            mPThread->start();
        }

        return 0;
    }

    int ffmpegAudioFilter::push(std::unique_ptr<IAFFrame> &frame, uint64_t timeOut)
    {
//        AF_LOGD("mInPut size is %d\n",mInPut.read_available());
//        AF_LOGD("mOutPut size is %d\n",mOutPut.read_available());
        if (mInPut.read_available() >= MAX_INPUT_BUFFER_COUNT
                || mOutPut.read_available() > MAX_INPUT_BUFFER_COUNT) {
            return -EAGAIN;
        }

        int64_t pts = frame->getInfo().pts;

        if (pts != INT64_MIN) {
            if (mLastInputPts != INT64_MIN) {
                int64_t deltaPts = pts - (mLastInputPts + mLastInPutDuration);

                if (llabs(deltaPts) > mLastInPutDuration / 2) {
                    mDeltaPts += deltaPts;
                    AF_LOGD("mDeltaPts is %lld\n", mDeltaPts);
                }
            }

            mLastInputPts = pts;
        }

        mLastInPutDuration = static_cast<int64_t>(frame->getInfo().audio.nb_samples * 1000 /
                             (float) (frame->getInfo().audio.sample_rate / 1000));
        mPts.push(pts);
        mInPut.push(frame.release());
        return 0;
    }

    int ffmpegAudioFilter::FilterLoop()
    {
        int ret;
        IAFFrame *frame = nullptr;

        if (!mInPut.empty()) {
            frame = mInPut.front();
            mInPut.pop();
        } else {
            af_usleep(10000);
            return 0;
        }

        if (frame == nullptr) {
            return 0;
        }

        IAFFrame::audioInfo info = frame->getInfo().audio;

        if (info.sample_rate != mSrcFormat.sample_rate
                || info.format != mSrcFormat.format
                || info.channels != mSrcFormat.channels) {
            /* reset the filer will drop the data in filter cache, so it is not expected
             */
            assert(0);
            mSrcFormat = info;

            if (m_pFilterGraph) {
                avfilter_graph_free(&m_pFilterGraph);
            }
        }

        {
            std::lock_guard<std::mutex> uMutex(mMutexRate);

            if (m_pFilterGraph == nullptr) {
                ret = init();

                if (ret < 0) {
                    AF_LOGE("init error\n");
                    return ret;
                }
            }
        }

        int64_t pts = dynamic_cast<AVAFFrame *>(frame)->getInfo().pts;

        if (mFirstPts == INT64_MIN) {
            mFirstPts = pts;
        }

        ret = av_buffersrc_add_frame(mAbuffer_ctx, getAVFrame(frame));

        if (ret < 0) {
            AF_LOGE("Error submitting the frame to the filtergraph: %s", getErrorString(ret));
            delete frame;
            return ret;
        }

        /* Get all the filtered output that is available. */
        while (true) {
            AVFrame *avFrame = nullptr;

            if (frame == nullptr) {
                avFrame = av_frame_alloc();
                frame = new AVAFFrame(&avFrame);
            }

            avFrame = getAVFrame(frame);
            avFrame->pts = pts;
            ret = av_buffersink_get_frame(mAbuffersink_ctx, avFrame);

            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                delete frame;
                break;
            }

            if (ret < 0) {
                delete frame;
                return ret;
            }

            AVRational tb = av_buffersink_get_time_base(mAbuffersink_ctx);

            if (avFrame->pts != AV_NOPTS_VALUE) {
                avFrame->pts = av_rescale_q(avFrame->pts, tb, AV_TIME_BASE_Q);
            }

            if (mReferInputPTS) {
                if (!mPts.empty()) {
                    pts = mPts.front();
                    mPts.pop();
                }

                avFrame->pts = pts;
            } else if (avFrame->pts != AV_NOPTS_VALUE) {
                avFrame->pts *= mRate;

                if (mFirstPts != INT64_MIN) {
                    avFrame->pts += mFirstPts;
                }

                avFrame->pts += mDeltaPts;
            }

            auto *tmp = dynamic_cast<AVAFFrame *> (frame);

            if (tmp) {
                tmp->updateInfo();
            }

#ifdef DUMP_PCM
            // int planar = av_sample_fmt_is_planar((enum AVSampleFormat) filt_frame->format);
            // int channels = av_get_channel_layout_nb_channels(filt_frame->channel_layout);
            // int planes = planar ? channels : 1;
            // int bps = av_get_bytes_per_sample((enum AVSampleFormat) filt_frame->format);
            // int plane_size = bps * filt_frame->nb_samples * (planar ? 1 : channels);
            // write(fd, filt_frame->extended_data[0], plane_size);
#endif
            mOutPut.push(frame);
            frame = nullptr;
        }

        return 0;
    }

    int ffmpegAudioFilter::pull(unique_ptr<IAFFrame> &frame, uint64_t timeOut)
    {
        if (mOutPut.empty()) {
            return -EAGAIN;
        }

        frame = unique_ptr<IAFFrame>(mOutPut.front());
        mOutPut.pop();
        return 0;
    }

    void ffmpegAudioFilter::flush()
    {
        if (mPThread) {
            mPThread->pause();
        }

        while (!mPts.empty()) {
            mPts.pop();
        }

        while (!mInPut.empty()) {
            delete mInPut.front();
            mInPut.pop();
        }

        while (!mOutPut.empty()) {
            delete mOutPut.front();
            mOutPut.pop();
        }

        if (m_pFilterGraph) {
            avfilter_graph_free(&m_pFilterGraph);
        }

        if (mPThread) {
            mPThread->start();
        }
    }

}
