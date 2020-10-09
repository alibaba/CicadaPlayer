//
// Created by moqi on 2020/5/28.
//
#define LOG_TAG "ffmpegVideoFilter"
#include "ffmpegVideoFilter.h"
#include <base/media/AVAFPacket.h>
#include <utils/frame_work_log.h>
#include <utils/timer.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
}
using namespace Cicada;
using namespace std;
#define MAX_INPUT_BUFFER_COUNT 2
#define MAX_OUTPUT_BUFFER_COUNT 2
ffmpegVideoFilter::ffmpegVideoFilter(const IVideoFilter::format &srcFormat, const IVideoFilter::format &dstFormat, bool active)
    : IVideoFilter(srcFormat, dstFormat, active)
{
    avfilter_register_all();
}
ffmpegVideoFilter::~ffmpegVideoFilter()
{
    delete mPThread;
    avfilter_graph_free(&m_pFilterGraph);
    flush();
}
bool ffmpegVideoFilter::setOption(const string &key, const string &value, const string &capacity)
{
    return false;
}
int ffmpegVideoFilter::init()
{
    char args[512];
    int ret = 0;
    const AVFilter *buffersrc = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();
    AVRational time_base = {100000, 1};//fmt_ctx->streams[video_stream_index]->time_base;
    enum AVPixelFormat pix_fmts[] = {AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE};

    m_pFilterGraph = avfilter_graph_alloc();
    if (!outputs || !inputs || !m_pFilterGraph) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    snprintf(args, sizeof(args), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d", mSrcFormat.width, mSrcFormat.height,
             mSrcFormat.format, time_base.num, time_base.den,
             // TODO: sar
             1, 1);

    ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in", args, NULL, m_pFilterGraph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source\n");
        goto end;
    }

    /* buffer video sink: to terminate the filter chain. */
    ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out", NULL, NULL, m_pFilterGraph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
        goto end;
    }

    ret = av_opt_set_int_list(buffersink_ctx, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format\n");
        goto end;
    }

    /*
     * Set the endpoints for the filter graph. The filter_graph will
     * be linked to the graph described by filters_descr.
     */

    /*
     * The buffer source output must be connected to the input pad of
     * the first filter described by filters_descr; since the first
     * filter input label is not specified, it is set to "in" by
     * default.
     */
    outputs->name = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    /*
     * The buffer sink input must be connected to the output pad of
     * the last filter described by filters_descr; since the last
     * filter output label is not specified, it is set to "out" by
     * default.
     */
    inputs->name = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    if ((ret = avfilter_graph_parse_ptr(m_pFilterGraph, mOpt.c_str(), &inputs, &outputs, NULL)) < 0) {
        goto end;
    }

    if ((ret = avfilter_graph_config(m_pFilterGraph, NULL)) < 0) {
        goto end;
    }
    if (mActive && mPThread == nullptr) {
        mPThread = NEW_AF_THREAD(FilterLoop);
        mPThread->start();
    }

end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    return ret;
    ;
}
int ffmpegVideoFilter::push(unique_ptr<IAFFrame> &frame, uint64_t timeOut)
{
    if (mInPut.size() >= MAX_INPUT_BUFFER_COUNT || mOutPut.size() > MAX_OUTPUT_BUFFER_COUNT) {
        if (!mActive) {
            FilterLoop();
        }
        return -EAGAIN;
    }
    mInPut.push(frame.release());
    if (!mActive) {
        FilterLoop();
    }

    return 0;
}
int ffmpegVideoFilter::pull(unique_ptr<IAFFrame> &frame, uint64_t timeOut)
{
    if (mOutPut.empty()) {
        return -EAGAIN;
    }

    frame = unique_ptr<IAFFrame>(mOutPut.front());
    mOutPut.pop();
    return 0;
}
void ffmpegVideoFilter::flush()
{
    if (mPThread) {
        mPThread->pause();
    }

    //    while (!mPts.empty()) {
    //        mPts.pop();
    //    }

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
int ffmpegVideoFilter::FilterLoop()
{
    int ret;
    IAFFrame *frame;

    while (!mInPut.empty() && mOutPut.read_available() < MAX_OUTPUT_BUFFER_COUNT) {
        frame = mInPut.front();
        mInPut.pop();
        IAFFrame::videoInfo info = frame->getInfo().video;

        if (info.width != mSrcFormat.width || info.format != mSrcFormat.format || info.height != mSrcFormat.height) {
            /* reset the filer will drop the data in filter cache, so it is not expected
         */
            assert(0);
            mSrcFormat = info;

            if (m_pFilterGraph) {
                avfilter_graph_free(&m_pFilterGraph);
            }
        }

        {
            //   std::lock_guard<std::mutex> uMutex(mMutexRate);

            if (m_pFilterGraph == nullptr) {
                ret = init();

                if (ret < 0) {
                    AF_LOGE("init error\n");
                    return ret;
                }
            }
        }

        int64_t pts = dynamic_cast<AVAFFrame *>(frame)->getInfo().pts;

        //        if (mFirstPts == INT64_MIN) {
        //            mFirstPts = pts;
        //        }

        ret = av_buffersrc_add_frame(buffersrc_ctx, getAVFrame(frame));

        if (ret < 0) {
            //   AF_LOGE("Error submitting the frame to the filtergraph: %s", getErrorString(ret));
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
            ret = av_buffersink_get_frame(buffersink_ctx, avFrame);

            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                delete frame;
                break;
            }

            if (ret < 0) {
                delete frame;
                return ret;
            }

            AVRational tb = av_buffersink_get_time_base(buffersink_ctx);

            if (avFrame->pts != AV_NOPTS_VALUE) {
                avFrame->pts = av_rescale_q(avFrame->pts, tb, av_get_time_base_q());
            }

            //            if (mReferInputPTS) {
            //                if (!mPts.empty()) {
            //                    pts = mPts.front();
            //                    mPts.pop();
            //                }
            //
            //                avFrame->pts = pts;
            //            } else if (avFrame->pts != AV_NOPTS_VALUE) {
            //                avFrame->pts *= mRate;
            //
            //                if (mFirstPts != INT64_MIN) {
            //                    avFrame->pts += mFirstPts;
            //                }
            //
            //                avFrame->pts += mDeltaPts;
            //            }

            auto *tmp = dynamic_cast<AVAFFrame *>(frame);

            if (tmp) {
                tmp->updateInfo();
            }

            mOutPut.push(frame);
            frame = nullptr;
        }
    }

    if (mActive) {
        af_msleep(10);
    }

    return 0;
}
