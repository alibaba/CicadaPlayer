//
// Created by lifujun on 2019/3/1.
//

#include <utils/frame_work_log.h>
#include <utils/ffmpeg_utils.h>
#include <utils/af_string.h>
#include "MetaToCodec.h"

extern "C" {
#include <libavutil/channel_layout.h>
}


void MetaToCodec::videoMetaToStream(AVStream *st, Stream_meta *meta)
{
    if (st == nullptr || meta == nullptr) {
        return;
    }

    AF_LOGD("videoMetaToStream...\n");
    st->codecpar->height = meta->height;
    st->codecpar->width = meta->width;
    st->codecpar->codec_tag = meta->codec_tag;
    st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    st->codecpar->codec_id = static_cast<AVCodecID>(CodecID2AVCodecID(meta->codec));
    int ret = av_dict_set(&st->metadata, "rotate", AfString::to_string(meta->rotate).c_str(), 0);

    if (ret < 0) {
        AF_LOGE("set rotate fail");
    }

    if (meta->extradata_size > 0) {
        st->codecpar->extradata = static_cast<uint8_t *>(malloc(
                                      meta->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE));
        memcpy(st->codecpar->extradata, meta->extradata, meta->extradata_size);
        st->codecpar->extradata_size = meta->extradata_size;
    }

    // TODO: get it
    AVRational r = {1, 1};
    st->codecpar->sample_aspect_ratio = r;
    /* take first format from list of supported formats */
    // TODO: get it
    st->codecpar->format = AV_PIX_FMT_YUV420P;
    st->avg_frame_rate = {(int) meta->avg_fps, 1};
    /* video time_base can be set to whatever is handy and supported by encoder */
    st->time_base = {1, AV_TIME_BASE};
    //拷贝到codecpar
}

void MetaToCodec::audioMetaToStream(AVStream *st, Stream_meta *meta)
{
    if (meta == nullptr || st == nullptr) {
        return;
    }

    AF_LOGD("audioMetaToStream...\n");
    st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
    st->codecpar->codec_id = static_cast<AVCodecID>(CodecID2AVCodecID(meta->codec));
    st->codecpar->sample_rate = meta->samplerate;
    st->codecpar->channel_layout = (uint64_t) av_get_channel_layout_nb_channels(static_cast<uint64_t>(meta->channels));
    st->codecpar->channels = meta->channels;
    /* take first format from list of supported formats */
    st->codecpar->format = (enum AVSampleFormat) meta->sample_fmt;
    st->time_base = {1, st->codecpar->sample_rate};
    st->codecpar->frame_size = meta->frame_size;

//        if (st->codec->codec_id == AV_CODEC_ID_AAC){
//            // TODO: check adts header
//           // pHandle->aac_bsf = av_bitstream_filter_init("aac_adtstoasc");
//            ff_stream_add_bitstream_filter(st, "aac_adtstoasc", NULL);
//        }
    if (meta->extradata_size > 0) {
        st->codecpar->extradata = static_cast<uint8_t *>(malloc(
                                      meta->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE));
        memcpy(st->codecpar->extradata, meta->extradata, meta->extradata_size);
        st->codecpar->extradata_size = meta->extradata_size;
    }
}
