//
// Created by moqi on 2019/11/7.
//

#include "ISampleDecrypt2c.h"
#include "utils/ffmpeg_utils.h"
#include <assert.h>
#include <libavformat/avformat.h>
#include <libavutil/avassert.h>
#include <libavutil/opt.h>

void avpriv_set_pts_info(AVStream *s, int pts_wrap_bits,
                         unsigned int pts_num, unsigned int pts_den);

typedef struct sampleDecryptContext {
    const AVClass *class;
    AVFormatContext *ctx;
    uint64_t decryptor;
    const char *subFormat;
    uint64_t *decrptyHandle;
} sampleDecryptContext;

#if (LIBAVFORMAT_VERSION_MAJOR < 58)
    static int sampleDecrypt_probe(AVProbeData *p)
#else
    static int sampleDecrypt_probe(const AVProbeData *p)
#endif
{
    // use avformat_open_input spec this model,can't probe any thing
    return 0;
}


static int update_streams_from_subdemuxer(AVFormatContext *s, AVStream *ist)
{
    int ret;
    AVStream *st = avformat_new_stream(s, NULL);
    st->need_parsing = ist->need_parsing;
    st->discard = ist->discard;
    // TODO: copy other useful data
//    st->sample_aspect_ratio = ist->sample_aspect_ratio;
//    st->time_base = ist->time_base;
//    st->avg_frame_rate = ist->avg_frame_rate;
//    st->duration = ist->duration;
    st->disposition = ist->disposition;
//    st->attached_pic              = ist->attached_pic ;
//    st->attached_pic.stream_index = ist->attached_pic.stream_index;
//    st->attached_pic.flags        = ist->attached_pic.flags;
    ret = avcodec_parameters_copy(st->codecpar, ist->codecpar);
    avpriv_set_pts_info(st, ist->pts_wrap_bits, (unsigned int) ist->time_base.num,
                        (unsigned int) ist->time_base.den);
    av_dict_copy(&st->metadata, ist->metadata, 0);
    return ret;
}

static int sampleDecrypt_read_header(AVFormatContext *s)
{
    int ret = 0;
    int i;
    sampleDecryptContext *c = s->priv_data;
    AVInputFormat *in_fmt = NULL;
    c->ctx = avformat_alloc_context();
    // TODO:  impl pb usage
    c->ctx->pb = s->pb;
    c->ctx->io_open = s->io_open;
    c->ctx->flags = s->flags;

    if (c->ctx->pb->seekable) {
        avio_seek(s->pb, 0, SEEK_SET);
    }

    c->ctx->interrupt_callback = s->interrupt_callback;
    ret = avformat_open_input(&c->ctx, s->filename, in_fmt, NULL);

    if (ret < 0) {
        av_log(s, AV_LOG_ERROR, "avformat_open_input error %d %s\n", ret,  getErrorString(ret));
        goto fail;
    }

    ret = avformat_find_stream_info(c->ctx, NULL);
    c->subFormat = av_strdup(c->ctx->iformat->name);
    av_log(s, AV_LOG_DEBUG, "subFormat is %s\n", c->subFormat);
    av_log(s, AV_LOG_ERROR, "have %d streams", c->ctx->nb_streams);
    av_dump_format(c->ctx, 0, s->filename, 0);

    for (i = 0; i < c->ctx->nb_streams; i++) {
        update_streams_from_subdemuxer(s, c->ctx->streams[i]);
    }

    s->ctx_flags = c->ctx->ctx_flags;
    s->duration = c->ctx->duration;
    return 0;
fail:
    avformat_close_input(&c->ctx);
    return ret;
}

static int sampleDecrypt_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    sampleDecryptContext *c = s->priv_data;
    int ret;
    ret = av_read_frame(c->ctx, pkt);

    if (ret < 0) {
        if (ret != -EAGAIN) {
            av_log(s, AV_LOG_ERROR, "read subdemuxer error %d (%s)\n", ret, getErrorString(ret));
        }
        //    s->pb->error = 0;//c->ctx->pb->error;
        return ret;
    }

    //update ctx_flags from subDemuxer
    s->ctx_flags = c->ctx->ctx_flags;
    s->duration = c->ctx->duration;

    //   av_log(s,AV_LOG_ERROR,"read a pkt %d\n",pkt->stream_index);
    if (pkt->stream_index >= s->nb_streams) {
        av_log(s, AV_LOG_ERROR, "add stream %d after read header\n", pkt->stream_index);
        update_streams_from_subdemuxer(s, c->ctx->streams[pkt->stream_index]);
    }

    if (c->decryptor) {
        int size = SampleDecryptDec((void *) c->decryptor, s->streams[pkt->stream_index]->codecpar->codec_id, pkt->data, pkt->size);
        assert(size > 0);
        if (size <= 0) {
            av_log(s, AV_LOG_ERROR, "SampleDecryptDec error\n");
        } else {
            pkt->size = size;
        }
    }

    return ret;
}

static int sampleDecrypt_read_seek(AVFormatContext *s, int stream_index,
                                   int64_t ts, int flags)
{
    sampleDecryptContext *c = s->priv_data;
    av_log(s, AV_LOG_ERROR, "seek to % " PRId64 "\n", ts);

    if (c->ctx->iformat->read_seek) {
        return c->ctx->iformat->read_seek(c->ctx, stream_index, ts, flags);
    }

    return 0;
}

static int sampleDecrypt_read_close(AVFormatContext *s)
{
    sampleDecryptContext *c = s->priv_data;
    av_log(s, AV_LOG_DEBUG, "sampleDecrypt_read_close \n");
    avformat_close_input(&c->ctx);
    return 0;
}


#define OFFSET(x) offsetof(sampleDecryptContext, x)
#define VD AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_DECODING_PARAM
static const AVOption options[] = {
    {
        "Decryptor", "Decryptor address", OFFSET(decryptor), AV_OPT_TYPE_UINT64, {.i64 = 0}, INT64_MIN, INT64_MAX, VD
    },

    {NULL}
};

static const AVClass sampleDecrypt_class = {
    .class_name = "sampleDecryptDec",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};
AVInputFormat sampleDecrypt_demuxer = {
    .name           = "sampleDecryp",
    //     .long_name      = NULL_IF_CONFIG_SMALL("FLV (Flash Video)"),
    .priv_data_size = sizeof(sampleDecryptContext),
    .read_probe     = sampleDecrypt_probe,
    .read_header    = sampleDecrypt_read_header,
    .read_packet    = sampleDecrypt_read_packet,
    .read_seek      = sampleDecrypt_read_seek,
    .read_close     = sampleDecrypt_read_close,
    .priv_class     = &sampleDecrypt_class,
};
