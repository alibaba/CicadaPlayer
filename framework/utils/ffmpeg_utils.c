//
// Created by moqi on 2019-07-08.
//

#include "ffmpeg_utils.h"
#include <assert.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/h264_parse.h>
#include <libavcodec/h264_ps.h>
#include <libavcodec/hevc_parse.h>
#include <libavcodec/hevc_ps.h>
#include <libavcodec/hevc_sei.h>
#include <libavformat/avc.h>
#include <libavformat/avformat.h>
#include <libavformat/avio_internal.h>
#include <libavformat/hevc.h>
#include <libavformat/internal.h>
#include <libavcodec/internal.h>
#include <libavutil/internal.h>
#include <libavutil/avstring.h>
#include <libavutil/intreadwrite.h>
#include <libavutil/timestamp.h>
#include <pthread.h>
#include <utils/frame_work_log.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t *creat_mutex()
{
    pthread_mutex_t *pMute = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));

    if (pMute == NULL) {
        return NULL;
    }

    pthread_mutex_init(pMute, NULL);
    return pMute;
}

static int lock_mutex(pthread_mutex_t *pMute)
{
    return -pthread_mutex_lock(pMute);
}

static int unlock_mutex(pthread_mutex_t *pMute)
{
    return pthread_mutex_unlock(pMute);
}

static void destroy_mutex(pthread_mutex_t **pMute)
{
    pthread_mutex_destroy(*pMute);
    free(*pMute);
    *pMute = NULL;
}

static int lockmgr(void **mtx, enum AVLockOp op)
{
    switch (op) {
        case AV_LOCK_CREATE:
            *mtx = creat_mutex();

            if (!*mtx) {
                return 1;
            }

            return 0;

        case AV_LOCK_OBTAIN:
            return lock_mutex(*mtx) != 0;

        case AV_LOCK_RELEASE:
            return unlock_mutex(*mtx) != 0;

        case AV_LOCK_DESTROY:
            destroy_mutex((pthread_mutex_t **) mtx);
            return 0;

        default:
            break;
    }

    return 1;
}

static void ffmpeg_log_back(void *ptr, int level, const char *fmt, va_list vl)
{
    static char line[1024];
    static int print_prefix = 1;

    if (level > av_log_get_level()) {
        return;
    }

    pthread_mutex_lock(&mutex);
    av_log_format_line(ptr, level, fmt, vl, line, sizeof(line), &print_prefix);
    pthread_mutex_unlock(&mutex);
    __log_print(AF_LOG_LEVEL_DEBUG, "FFMPEG", "%s", line);
}

static void ffmpeg_init_once()
{
    AF_LOGI("Ffmpeg version %s", av_version_info());
    av_lockmgr_register(lockmgr);
    av_log_set_level(AV_LOG_INFO);
    av_log_set_callback(ffmpeg_log_back);
    av_register_all();
    avformat_network_init();
}

void ffmpeg_init()
{
    static pthread_once_t once;
    pthread_once(&once, ffmpeg_init_once);
}

void ffmpeg_deinit()
{
    av_lockmgr_register(NULL);
    avformat_network_deinit();
}

int AVDictionary2SourceMeta(Source_meta **meta, const AVDictionary *dictionary)
{
    int count = 0;
    Source_meta *head = NULL;
    Source_meta *tail = NULL;
    AVDictionaryEntry *t = NULL;
    t = av_dict_get(dictionary, "", t, AV_DICT_IGNORE_SUFFIX);

    while (t) {
        Source_meta *node = malloc(sizeof(Source_meta));
        node->key = strdup(t->key);
        node->value = strdup(t->value);
        node->next = NULL;

        if (head == NULL) {
            head = node;
            tail = head;
        } else {
            tail->next = node;
            tail = node;
        }

        count++;
        t = av_dict_get(dictionary, "", t, AV_DICT_IGNORE_SUFFIX);
    }

    *meta = head;
    return count;
}

enum AFCodecID AVCodec2CicadaCodec(enum AVCodecID codec)
{
    switch (codec) {
        /*Audio*/
        case AV_CODEC_ID_AAC:
                case AV_CODEC_ID_AAC_LATM:
                        return AF_CODEC_ID_AAC;

//        case AV_CODEC_ID_DTS:
//            return AF_CODEC_ID_DTS;

        //case AV_CODEC_ID_DTSH:
        //    return AF_CODEC_ID_DTSE;

        case AV_CODEC_ID_AC3:
            return AF_CODEC_ID_AC3;

//
        case AV_CODEC_ID_EAC3:
            return AF_CODEC_ID_EAC3;

        case AV_CODEC_ID_MP3:
            return AF_CODEC_ID_MP3;

        case AV_CODEC_ID_MP2:
            return AF_CODEC_ID_MP2;

        case AV_CODEC_ID_MP1:
            return AF_CODEC_ID_MP1;

//        case AV_CODEC_ID_APE:
//            return AF_CODEC_ID_APE;
//
//        case AV_CODEC_ID_QDM2:
//            return AF_CODEC_ID_QDM2;
//
//        case AV_CODEC_ID_COOK:
//            return AF_CODEC_ID_COOK;
//
//        case AV_CODEC_ID_SIPR:
//            return AF_CODEC_ID_SIPR;
//
//        case AV_CODEC_ID_AMR_NB:
//            return AF_CODEC_ID_AMR_NB;
//
//        case AV_CODEC_ID_WMAV2:
//            return AF_CODEC_ID_WMAV2;
//
//        case AV_CODEC_ID_WMAPRO:
//            return AF_CODEC_ID_WMAPRO;

        case AV_CODEC_ID_PCM_S16LE:
            return AF_CODEC_ID_PCM_S16LE;

        case AV_CODEC_ID_PCM_S16BE:
            return AF_CODEC_ID_PCM_S16BE;

//        case AV_CODEC_ID_PCM_BLURAY:
//            return AF_CODEC_ID_PCM_BLURAY;

//        case AV_CODEC_ID_PCM_S24LE:
//            return AF_CODEC_ID_PCM_S24LE;

        case AV_CODEC_ID_PCM_U8:
            return AF_CODEC_ID_PCM_U8;

//        case AV_CODEC_ID_PCM_MULAW:
//            return AF_CODEC_ID_PCM_MULAW;
//
//        case AV_CODEC_ID_ADPCM_IMA_WAV:
//        case AV_CODEC_ID_ADPCM_MS:
//            return AF_CODEC_ID_ADPCM;
//
//        case AV_CODEC_ID_ATRAC3:
//            return AF_CODEC_ID_ATRAC3;
//
//        case AV_CODEC_ID_VORBIS:
//            return AF_CODEC_ID_VORBIS;
//
//        case AV_CODEC_ID_ALAC:
//            return AF_CODEC_ID_ALAC;
//
//        case AV_CODEC_ID_FLAC:
//            return AF_CODEC_ID_FLAC;

        case AV_CODEC_ID_OPUS:
            return AF_CODEC_ID_OPUS;

        /*Video*/
        case AV_CODEC_ID_H264:
            return AF_CODEC_ID_H264;

//        case AV_CODEC_ID_MPEG2VIDEO:
//            return AF_CODEC_ID_MPEG2VIDEO;

        case AV_CODEC_ID_MPEG4:
            return AF_CODEC_ID_MPEG4;

//        case AV_CODEC_ID_MSMPEG4V2:
//            return AF_CODEC_ID_MSMPEG4V2;
//
//        case AV_CODEC_ID_MSMPEG4V3:
//            if (codec->codec_tag == MKTAG('D', 'I', 'V', '3')) {
//                return AF_CODEC_ID_DIV311;
//            }
//
//        case AV_CODEC_ID_FLV1:
//            return AF_CODEC_ID_FLV1;
//
//        case AV_CODEC_ID_SVQ3:
//            return AF_CODEC_ID_SVQ3;
//
//        case AV_CODEC_ID_RV40:
//            return AF_CODEC_ID_RV40;
//
//        case AV_CODEC_ID_RV30:
//            return AF_CODEC_ID_RV30;
//
//        case AV_CODEC_ID_VC1:
//            return AF_CODEC_ID_VC1;
//
//        case AV_CODEC_ID_WMV3:
//            return AF_CODEC_ID_WMV3;
//
//        case AV_CODEC_ID_WMV1:
//            return AF_CODEC_ID_WMV1;
//
//        case AV_CODEC_ID_WMV2:
//            return AF_CODEC_ID_WMV2;
//
//        case AV_CODEC_ID_MPEG1VIDEO:
//            return AF_CODEC_ID_MPEG1VIDEO;
//
//        case AV_CODEC_ID_VP6:
//        case AV_CODEC_ID_VP6F:
//            return AF_CODEC_ID_VP6;
//
        case AV_CODEC_ID_VP8:
            return AF_CODEC_ID_VP8;

        case AV_CODEC_ID_VP9:
            return AF_CODEC_ID_VP9;

//
//        case AV_CODEC_ID_MJPEG:
//            return AF_CODEC_ID_MJPEG;
//
//        case AV_CODEC_ID_H263 :
//            return AF_CODEC_ID_H263;

        case AV_CODEC_ID_HEVC:
            return AF_CODEC_ID_HEVC;

        case AV_CODEC_ID_AV1:
            return AF_CODEC_ID_AV1;

        /* subtitle */
        case AV_CODEC_ID_WEBVTT:
            return AF_CODEC_ID_WEBVTT;
            //
            //        case AV_CODEC_ID_SSA:
            //            return AF_CODEC_ID_SSA;
            //
            //        case AV_CODEC_ID_SRT:
            //            return AF_CODEC_ID_SRT;

        default:
//            if (codec->codec_id == AV_CODEC_ID_NONE && (!av_strcasecmp((char *) &codec->codec_tag, "dtse")))
//                return AF_CODEC_ID_DTSE;
            break;
    }

//    av_log(NULL, AV_LOG_ERROR, "unsupport codec id %d tag %.4s\n", codec->codec_id, (char *) &codec->codec_tag);
    return AF_CODEC_ID_NONE;
}

typedef struct codec_pair_t {
    enum AFCodecID klId;
    enum AVCodecID avId;
} codec_pair;

static codec_pair codec_pair_table[] = {
    {AF_CODEC_ID_AAC,       AV_CODEC_ID_AAC},
    {AF_CODEC_ID_AC3,       AV_CODEC_ID_AC3},
    {AF_CODEC_ID_EAC3,      AV_CODEC_ID_EAC3},
    {AF_CODEC_ID_MP3,       AV_CODEC_ID_MP3},
    {AF_CODEC_ID_MP2,       AV_CODEC_ID_MP2},
    {AF_CODEC_ID_MP1,       AV_CODEC_ID_MP1},
    {AF_CODEC_ID_PCM_S16LE, AV_CODEC_ID_PCM_S16LE},
//    {AF_CODEC_ID_PCM_MULAW, AV_CODEC_ID_PCM_MULAW},
//    {AF_CODEC_ID_APE,       AV_CODEC_ID_APE},
//    {AF_CODEC_ID_FLAC,      AV_CODEC_ID_FLAC},
    {AF_CODEC_ID_H264,      AV_CODEC_ID_H264},
    {AF_CODEC_ID_HEVC,      AV_CODEC_ID_HEVC},
    {AF_CODEC_ID_AV1,       AV_CODEC_ID_AV1},
    {AF_CODEC_ID_VP8,       AV_CODEC_ID_VP8},
    {AF_CODEC_ID_VP9,       AV_CODEC_ID_VP9},
    {AF_CODEC_ID_OPUS,      AV_CODEC_ID_OPUS},
    {AF_CODEC_ID_MPEG4,     AV_CODEC_ID_MPEG4},
    {AF_CODEC_ID_NONE,      AV_CODEC_ID_NONE},
};

enum AVCodecID CodecID2AVCodecID(enum AFCodecID codec)
{
    int num = sizeof(codec_pair_table) / sizeof(codec_pair_table[0]);
    int i;

    for (i = 0; i < num; i++) {
        if (codec_pair_table[i].klId == codec) {
            return codec_pair_table[i].avId;
        }
    }

    AF_LOGD("cicada codec %d not found\n", codec);
    return AV_CODEC_ID_NONE;
}


typedef struct pix_fmt_pair_t {
    enum AFPixelFormat klId;
    enum AVPixelFormat avId;
} pix_fmt_pair;

static pix_fmt_pair pix_fmt_pair_table[] = {
        {AF_PIX_FMT_NONE, AV_PIX_FMT_NONE},
        {AF_PIX_FMT_YUV420P, AV_PIX_FMT_YUV420P},
        {AF_PIX_FMT_YUV422P, AV_PIX_FMT_YUV422P},
        {AF_PIX_FMT_YUVJ420P, AV_PIX_FMT_YUVJ420P},
        {AF_PIX_FMT_YUVJ422P, AV_PIX_FMT_YUVJ422P},
        {AF_PIX_FMT_YUV420P10BE, AV_PIX_FMT_YUV420P10BE},
        {AF_PIX_FMT_YUV420P10LE, AV_PIX_FMT_YUV420P10LE},
        {AF_PIX_FMT_D3D11, AV_PIX_FMT_D3D11},
        {AF_PIX_FMT_DXVA2_VLD, AV_PIX_FMT_DXVA2_VLD},
};

int AVPixFmt2Cicada(enum AVPixelFormat fmt)
{
    int num = sizeof(pix_fmt_pair_table) / sizeof(pix_fmt_pair_table[0]);
    int i;

    for (i = 0; i < num; i++) {
        if (pix_fmt_pair_table[i].avId == fmt) {
            return pix_fmt_pair_table[i].klId;
        }
    }

    AF_LOGD("AVPixelFormat %d not found\n", fmt);
    return AF_PIX_FMT_NONE;
}

int AVColorSpace2AF(enum AVColorSpace space)
{
    switch (space) {
        case AVCOL_SPC_BT470BG:
        case AVCOL_SPC_SMPTE170M:
            return COLOR_SPACE_BT601;

        case AVCOL_SPC_BT709:
            return COLOR_SPACE_BT709;

        case AVCOL_SPC_BT2020_NCL:
        case AVCOL_SPC_BT2020_CL:
            return COLOR_SPACE_BT2020;

        default:
            return COLOR_SPACE_UNSPECIFIED;
    }
}

int AVColorRange2AF(enum AVColorRange range)
{
    switch (range) {
        case AVCOL_RANGE_MPEG:
            return COLOR_RANGE_LIMITIED;

        case AVCOL_RANGE_JPEG:
            return COLOR_RANGE_FULL;

        default:
            return COLOR_RANGE_UNSPECIFIED;
    }
}

int set_stream_meta(struct AVStream *pStream, Stream_meta *meta)
{
    AVCodecParameters *codecpar = pStream->codecpar;

    switch (meta->type) {
        case STREAM_TYPE_VIDEO:
            if (meta->height > 0 && meta->width > 0) {
                codecpar->height = meta->height;
                codecpar->width = meta->width;
            }

            if (meta->pixel_fmt >= 0) {
                codecpar->format = meta->pixel_fmt;
            }

            pStream->r_frame_rate = av_d2q(meta->avg_fps, 1000);
            break;

        case STREAM_TYPE_AUDIO:
            if (meta->channels > 0) {
                codecpar->channels = meta->channels;
            }

            if (meta->samplerate > 0) {
                codecpar->sample_rate = meta->samplerate;
            }

            if (meta->sample_fmt > 0) {
                codecpar->format = meta->sample_fmt;
            }

            if (meta->frame_size > 0) {
                codecpar->frame_size = meta->frame_size;
            }

            break;

        default:
            break;
    }

    if (meta->extradata_size > 0 && meta->extradata) {
        if (codecpar->extradata) {
            free(codecpar->extradata);
        }

        codecpar->extradata = av_mallocz(meta->extradata_size + AVPROBE_PADDING_SIZE);
        memcpy(codecpar->extradata, meta->extradata, meta->extradata_size);
        codecpar->extradata_size = meta->extradata_size;
    }

    return 0;
}

int get_stream_meta(const struct AVStream *pStream, Stream_meta *meta)
{
    enum AVMediaType codec_type = pStream->codecpar->codec_type;
    memset(meta, 0, sizeof(Stream_meta));
    meta->type = STREAM_TYPE_UNKNOWN;
    meta->disposition = pStream->disposition;
    meta->attached_pic = meta->disposition & AV_DISPOSITION_ATTACHED_PIC;
    AVDictionary2SourceMeta(&meta->meta, pStream->metadata);
    AVDictionaryEntry *entry = av_dict_get(pStream->metadata, "title", NULL, 0);

    if (entry != NULL) {
        meta->title = entry->value;
    }

    entry = av_dict_get(pStream->metadata, "language", NULL, 0);

    if (entry != NULL) {
        meta->language = entry->value;
    }

//        entry = av_dict_get(pStream->metadata, "seeked_time", NULL, 0);
//
//        if (entry != NULL) {
//            meta->seeked_time = atol(entry->value);
//            pHandle->start_time = meta->seeked_time;
//        }
    meta->codec = AVCodec2CicadaCodec(pStream->codecpar->codec_id);
    meta->codec_tag = pStream->codecpar->codec_tag;
//    meta->cicada_codec_context = pStream->codec;
    meta->cicada_codec_context_size = sizeof(AVCodecContext);
//        meta->index = stream_index;
    meta->ptsTimeBase = (float) pStream->time_base.num * 1000000 / (float) pStream->time_base.den;
    if (codec_type == AVMEDIA_TYPE_VIDEO) {
        if (pStream->sample_aspect_ratio.num && av_cmp_q(pStream->sample_aspect_ratio, pStream->codecpar->sample_aspect_ratio)) {
            AVRational display_aspect_ratio;
            av_reduce(&display_aspect_ratio.num, &display_aspect_ratio.den,
                      pStream->codecpar->width * (int64_t) pStream->sample_aspect_ratio.num,
                      pStream->codecpar->height * (int64_t) pStream->sample_aspect_ratio.den, 1024 * 1024);
            meta->displayWidth = pStream->codecpar->width;
            meta->displayWidth *= pStream->sample_aspect_ratio.num;
            meta->displayWidth /= pStream->sample_aspect_ratio.den;
            meta->displayHeight = pStream->codecpar->height;
            AF_LOGI("DAR %d:%d", meta->displayWidth, meta->displayHeight);
        } else if (pStream->codecpar->sample_aspect_ratio.num){
            meta->displayWidth = pStream->codecpar->width;
            meta->displayWidth *= pStream->codecpar->sample_aspect_ratio.num;
            meta->displayWidth /= pStream->codecpar->sample_aspect_ratio.den;
            meta->displayHeight = pStream->codecpar->height;
            AF_LOGI("DAR %d:%d", meta->displayWidth, meta->displayHeight);
        }
        else {
            meta->displayWidth = meta->displayHeight = 0;
        }

        meta->type = STREAM_TYPE_VIDEO;
        meta->width = pStream->codecpar->width;
        meta->height = pStream->codecpar->height;
        meta->profile = pStream->codecpar->profile;
        meta->pixel_fmt = pStream->codecpar->format;
        meta->color_range = (enum AFColorRange) pStream->codecpar->color_range;
        meta->color_primaries = (enum AFColorPrimaries) pStream->codecpar->color_primaries;
        meta->color_space = (enum AFColorSpace) pStream->codecpar->color_space;
        meta->chroma_location = (enum AFChromaLocation) pStream->codecpar->chroma_location;
        meta->color_trc = (enum AFColorTransferCharacteristic) pStream->codecpar->color_trc;

        if (meta->codec == AF_CODEC_ID_H264) {
            meta->interlaced = InterlacedType_UNKNOWN;
        } else {
            meta->interlaced = InterlacedType_NO;
        }

        if (pStream->parser && meta->interlaced == InterlacedType_UNKNOWN) {
            if (pStream->parser->field_order == AV_FIELD_PROGRESSIVE
                    || pStream->parser->picture_structure == AV_PICTURE_STRUCTURE_FRAME) {
                meta->interlaced = InterlacedType_NO;
            } else if (pStream->parser->picture_structure != AV_PICTURE_STRUCTURE_UNKNOWN
                       || pStream->parser->field_order != AV_FIELD_UNKNOWN) {
                meta->interlaced = InterlacedType_YES;
            }
        }

        entry = av_dict_get(pStream->metadata, "rotate", NULL, 0);

        if (entry != NULL) {
            meta->rotate = atoi(entry->value);
            av_log(NULL, AV_LOG_ERROR, "rotate is %d\n", meta->rotate);
        } else {
            meta->rotate = 0;
        }

        if (pStream->avg_frame_rate.den && pStream->avg_frame_rate.num) {
            meta->avg_fps = av_q2d(pStream->avg_frame_rate);
        } else if (pStream->r_frame_rate.den && pStream->r_frame_rate.num) {
            meta->avg_fps = av_q2d(pStream->r_frame_rate);
        } else {
//            double tbn = 0.0f, tbc = 0.0f;
//
//            if (pStream->time_base.den && pStream->time_base.num)
//                tbn = 1 / av_q2d(pStream->time_base);
//
//            if (pStream->codec->time_base.den && pStream->codec->time_base.num)
//                tbc = 1 / av_q2d(pStream->codec->time_base);
//
//            if (tbn == tbc && tbn != 0.0f)
//                meta->avg_fps = tbn;
//            else
            meta->avg_fps = 0.0f;
        }
    } else if (codec_type == AVMEDIA_TYPE_AUDIO) {
        meta->type = STREAM_TYPE_AUDIO;
//            if (pStream->codecpar->codec_id == AV_CODEC_ID_AAC)
//                get_aac_profile(pStream->codecpar);
        meta->channels = pStream->codecpar->channels;
        meta->channel_layout = pStream->codecpar->channel_layout;
        meta->samplerate = pStream->codecpar->sample_rate;
        meta->frame_size = pStream->codecpar->frame_size;
        meta->profile = pStream->codecpar->profile;
        meta->bits_per_coded_sample = pStream->codecpar->bits_per_coded_sample;
        // TODO: conver sample_fmt
        meta->sample_fmt = (enum AFSampleFormat) pStream->codecpar->format;
    } else if (codec_type == AVMEDIA_TYPE_SUBTITLE) {
        meta->type = STREAM_TYPE_SUB;
    } else {
        meta->type = STREAM_TYPE_UNKNOWN;
    }

    meta->extradata_size = pStream->codecpar->extradata_size;
    meta->extradata = malloc(pStream->codecpar->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
    memcpy(meta->extradata, pStream->codecpar->extradata, pStream->codecpar->extradata_size);
    meta->nb_index_entries = pStream->nb_index_entries;
    meta->pid = pStream->id;
//        if (pHandle->have_program && pHandle->have_program[stream_index] == 0) {
//            meta->no_program = 1;
//        } else
//            meta->no_program = 0;
    return 0;
}

int af_get_bytes_per_sample(enum AFSampleFormat fmt)
{
    return av_get_bytes_per_sample((enum AVSampleFormat) fmt);
}


void copyYuvData_yuv420p(const AVFrame *frame, unsigned char *buffer, int width, int height)
{
    unsigned char *src = NULL;
    unsigned char *dst = buffer;
    int i = 0;

    for (i = 0; i < height; i++) {
        src = frame->data[0] + i * frame->linesize[0];
        memcpy(dst, src, width);
        dst += width;
    }

    for (i = 0; i < height / 2; i++) {
        src = frame->data[1] + i * frame->linesize[1];
        memcpy(dst, src, width / 2);
        dst += width / 2;
    }

    for (i = 0; i < height / 2; i++) {
        src = frame->data[2] + i * frame->linesize[2];
        memcpy(dst, src, width / 2);
        dst += width / 2;
    }
}

int getPCMDataLen(int channels, enum AVSampleFormat format, int nb_samples)
{
    return channels * av_get_bytes_per_sample(format) * nb_samples;
}

int getPCMFrameLen(const AVFrame *frame)
{
    int sampleSize = av_get_bytes_per_sample((enum AVSampleFormat) (frame->format));
    return frame->channels * sampleSize * frame->nb_samples;
}

int getPCMFrameDuration(const AVFrame *frame)
{
    return (int) (frame->nb_samples / ((double) (frame->sample_rate) / 1000000));
}

void copyPCMData(const AVFrame *frame, uint8_t *buffer)
{
    int offset = 0;
    int sampleSize = av_get_bytes_per_sample((enum AVSampleFormat) (frame->format));

    if (av_sample_fmt_is_planar((enum AVSampleFormat) frame->format)) {
        for (int i = 0; i < frame->nb_samples; i++) {
            for (int ch = 0; ch < frame->channels; ch++) {
                memcpy(buffer + offset, frame->data[ch] + sampleSize * i, sampleSize);
                offset += sampleSize;
            }
        }
    } else {
        memcpy(buffer, frame->extended_data[0], ((size_t) sampleSize * frame->nb_samples * frame->channels));
    }
}

size_t copyPCMDataWithOffset(const AVFrame *frame, int frameOffset, uint8_t *outBuffer, size_t outSize, bool *frameClear)
{
    int sampleSize = av_get_bytes_per_sample((enum AVSampleFormat) (frame->format));
    int totalWriteSize = 0;

    if (av_sample_fmt_is_planar((enum AVSampleFormat) frame->format)) {
        int samplesOffset = frameOffset / (frame->channels * sampleSize);
        int channelsOffset = (frameOffset % (frame->channels * sampleSize)) / frame->channels;
        int writeOffset = (frameOffset % sampleSize);

        for (int i = samplesOffset; i < frame->nb_samples; i++) {
            for (; channelsOffset < frame->channels; channelsOffset++) {
                if (outSize == totalWriteSize) {
                    *frameClear = false;
                    return outSize;
                } else if (outSize < (totalWriteSize + sampleSize)) {
                    memcpy(outBuffer + writeOffset + totalWriteSize, frame->data[channelsOffset] + sampleSize * i,
                           outSize - totalWriteSize);
                    *frameClear = false;
                    return outSize;
                }

                memcpy(outBuffer + writeOffset + totalWriteSize, frame->data[channelsOffset] + sampleSize * i, sampleSize);
                writeOffset = 0;
                totalWriteSize += sampleSize;
            }

            channelsOffset = 0;
        }

        *frameClear = true;
        return totalWriteSize;
    } else {
        if (outSize >= (sampleSize * frame->nb_samples * frame->channels - frameOffset)) {
            totalWriteSize = (sampleSize * frame->nb_samples * frame->channels - frameOffset);
            *frameClear = true;
        } else {
            totalWriteSize = outSize;
            *frameClear = false;
        }

        memcpy(outBuffer, (frame->extended_data[0]) + frameOffset, totalWriteSize);
        return totalWriteSize;
    }
}

void copyPCMData2(const AVFrame *frame, fillBufferCallback fillCallback, void *args)
{
    int sampleSize = av_get_bytes_per_sample((enum AVSampleFormat) (frame->format));

    if (av_sample_fmt_is_planar((enum AVSampleFormat) frame->format)) {
        for (int i = 0; i < frame->nb_samples; i++) {
            for (int ch = 0; ch < frame->channels; ch++) {
                if (fillCallback != NULL) {
                    fillCallback(args, frame->data[ch] + sampleSize * i, sampleSize);
                }
            }
        }
    } else {
        if (fillCallback != NULL) {
            fillCallback(args, frame->extended_data[0], ((size_t) sampleSize * frame->nb_samples * frame->channels));
        }
    }
}

bool updateH26xHeader2xxc(AVCodecParameters *par)
{
    uint8_t *extradata = par->extradata;
    int extradata_size = par->extradata_size;
    int ret;

    if (!extradata_size) {
        return false;
    }

    if (par->codec_id != AV_CODEC_ID_H264 && par->codec_id != AV_CODEC_ID_HEVC) {
        return false;
    }

    AVIOContext *pb;

    if (avio_open_dyn_buf(&pb) < 0) {
        return false;
    }

    if (par->codec_id == AV_CODEC_ID_H264) {
        ret = ff_isom_write_avcc(pb, extradata, extradata_size);
    } else {
        ret = ff_isom_write_hvcc(pb, extradata, extradata_size, 0);
    }

    if (ret < 0) {
        ffio_free_dyn_buf(&pb);
        return false;
    }

    extradata_size = avio_close_dyn_buf(pb, &extradata);

    if (extradata > 0) {
        av_free(par->extradata);
        par->extradata = av_realloc(extradata, extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
        par->extradata_size = extradata_size;
        return true;
    }

    return false;
}

static char errorBuff[256];

const char *getErrorString(int err)
{
    av_strerror(err, errorBuff, sizeof(errorBuff));
    return errorBuff;
}

int h2645_ps_to_nalu(const uint8_t *src, int src_size, uint8_t **out, int *out_size)
{
    int i;
    int ret = 0;
    uint8_t *p = NULL;
    static const uint8_t nalu_header[] = { 0x00, 0x00, 0x00, 0x01 };

    if (!out || !out_size) {
        return AVERROR(EINVAL);
    }

    p = av_malloc(sizeof(nalu_header) + src_size);
    if (!p) {
        return AVERROR(ENOMEM);
    }

    *out = p;
    *out_size = sizeof(nalu_header) + src_size;

    memcpy(p, nalu_header, sizeof(nalu_header));
    memcpy(p + sizeof(nalu_header), src, src_size);

    /* Escape 0x00, 0x00, 0x0{0-3} pattern */
    for (i = 4; i < *out_size; i++) {
        if (i < *out_size - 3 &&
            p[i + 0] == 0 &&
            p[i + 1] == 0 &&
            p[i + 2] <= 3) {
            uint8_t *new_data;

            *out_size += 1;
            new_data = av_realloc(*out, *out_size);
            if (!new_data) {
                ret = AVERROR(ENOMEM);
                goto done;
            }
            *out = p = new_data;

            i = i + 2;
            memmove(p + i + 1, p + i, *out_size - (i + 1));
            p[i] = 0x03;
        }
    }
    done:
    if (ret < 0) {
        av_freep(out);
        *out_size = 0;
    }

    return ret;
}

int parse_h264_extraData(enum AVCodecID codecId, const uint8_t* extraData,int extraData_size,
                         uint8_t** sps_data ,int*sps_data_size,
                         uint8_t** pps_data, int* pps_data_size,
                         int* nal_length_size
                         )
{
    AVCodec *codec = avcodec_find_decoder(codecId);
    if (codec == NULL) {
        return -1;
    }

    AVCodecContext *avctx = avcodec_alloc_context3((const AVCodec *) codec);
    if (avctx == NULL) {
        return -1;
    }

    int ret;

    H264ParamSets ps;
    const PPS *pps = NULL;
    const SPS *sps = NULL;
    int is_avc = 0;

    memset(&ps, 0, sizeof(ps));

    ret = ff_h264_decode_extradata((const uint8_t *) extraData, extraData_size, &ps, &is_avc, nal_length_size, 0, avctx);
    if (ret < 0) {
        goto done;
    }

    int i;
    for (i = 0; i < MAX_PPS_COUNT; i++) {
        if (ps.pps_list[i]) {
            pps = (const PPS *) ps.pps_list[i]->data;
            break;
        }
    }

    if (pps) {
        if (ps.sps_list[pps->sps_id]) {
            sps = (const SPS *) ps.sps_list[pps->sps_id]->data;
        }
    }

    if (pps && sps) {

        if ((ret = h2645_ps_to_nalu(sps->data, sps->data_size, sps_data, sps_data_size)) < 0) {
            goto done;
        }

        if ((ret = h2645_ps_to_nalu(pps->data, pps->data_size, pps_data, pps_data_size)) < 0) {
            goto done;
        }
    } else {
        av_log(avctx, AV_LOG_ERROR, "Could not extract PPS/SPS from extradata");
        ret = AVERROR_INVALIDDATA;
    }

    done:
    ff_h264_ps_uninit(&ps);
    avcodec_free_context(&avctx);
    return ret;
}

int parse_h265_extraData(enum AVCodecID codecId, const uint8_t* extradata,int extradata_size,
                         uint8_t** vps_data ,int*vps_data_size,
                         uint8_t** sps_data ,int*sps_data_size,
                         uint8_t** pps_data, int* pps_data_size,
                         int* nal_length_size)
{
#ifdef ENABLE_CODEC_HEVC
    AVCodec *codec = avcodec_find_decoder(codecId);
    if (codec == NULL) {
        return -1;
    }

    AVCodecContext *avctx = avcodec_alloc_context3((const AVCodec *) codec);
    if (avctx == NULL) {
        return -1;
    }

    int i;
    int ret;

    HEVCParamSets ps;
    HEVCSEI sei;

    const HEVCVPS *vps = NULL;
    const HEVCPPS *pps = NULL;
    const HEVCSPS *sps = NULL;
    int is_nalff = 0;

    memset(&ps, 0, sizeof(ps));
    memset(&sei, 0, sizeof(sei));

    ret = ff_hevc_decode_extradata(extradata, extradata_size, &ps, &sei, &is_nalff, nal_length_size, 0, 1, avctx);
    if (ret < 0) {
        goto done;
    }

    for (i = 0; i < HEVC_MAX_VPS_COUNT; i++) {
        if (ps.vps_list[i]) {
            vps = (const HEVCVPS *) ps.vps_list[i]->data;
            break;
        }
    }

    for (i = 0; i < HEVC_MAX_PPS_COUNT; i++) {
        if (ps.pps_list[i]) {
            pps = (const HEVCPPS *) ps.pps_list[i]->data;
            break;
        }
    }

    if (pps) {
        if (ps.sps_list[pps->sps_id]) {
            sps = (const HEVCSPS *) ps.sps_list[pps->sps_id]->data;
        }
    }

    if (vps && pps && sps) {
        if ((ret = h2645_ps_to_nalu(vps->data, vps->data_size, vps_data, vps_data_size)) < 0 ||
            (ret = h2645_ps_to_nalu(sps->data, sps->data_size, sps_data, sps_data_size)) < 0 ||
            (ret = h2645_ps_to_nalu(pps->data, pps->data_size, pps_data, pps_data_size)) < 0) {
            goto done;
        }
    } else {
        av_log(avctx, AV_LOG_ERROR, "Could not extract VPS/PPS/SPS from extradata");
        ret = AVERROR_INVALIDDATA;
    }

    done:
    ff_hevc_ps_uninit(&ps);
    avcodec_free_context(&avctx);
    return ret;
#else
    return -ENOSYS;
#endif
}
#define RELATIVE_TS_BASE (INT64_MAX - (1LL<<48))
static int is_relative(int64_t ts) {
    return ts > (RELATIVE_TS_BASE - (1LL<<48));
}
static int has_decode_delay_been_guessed(AVStream *st)
{
    if (st->codecpar->codec_id != AV_CODEC_ID_H264) return 1;
    if (!st->info) // if we have left find_stream_info then nb_decoded_frames won't increase anymore for stream copy
        return 1;
#if CONFIG_H264_DECODER
    if (st->internal->avctx->has_b_frames &&
       avpriv_h264_has_num_reorder_frames(st->internal->avctx) == st->internal->avctx->has_b_frames)
        return 1;
#endif
    if (st->internal->avctx->has_b_frames<3)
        return st->nb_decoded_frames >= 7;
    else if (st->internal->avctx->has_b_frames<4)
        return st->nb_decoded_frames >= 18;
    else
        return st->nb_decoded_frames >= 20;
}
static AVPacketList *get_next_pkt(AVFormatContext *s, AVStream *st, AVPacketList *pktl)
{
    if (pktl->next)
        return pktl->next;
    if (pktl == s->internal->packet_buffer_end)
        return s->internal->parse_queue;
    return NULL;
}

static int64_t select_from_pts_buffer(AVStream *st, int64_t *pts_buffer, int64_t dts) {
    int onein_oneout = st->codecpar->codec_id != AV_CODEC_ID_H264 &&
                       st->codecpar->codec_id != AV_CODEC_ID_HEVC;

    if(!onein_oneout) {
        int delay = st->internal->avctx->has_b_frames;
        int i;

        if (dts == AV_NOPTS_VALUE) {
            int64_t best_score = INT64_MAX;
            for (i = 0; i<delay; i++) {
                if (st->pts_reorder_error_count[i]) {
                    int64_t score = st->pts_reorder_error[i] / st->pts_reorder_error_count[i];
                    if (score < best_score) {
                        best_score = score;
                        dts = pts_buffer[i];
                    }
                }
            }
        } else {
            for (i = 0; i<delay; i++) {
                if (pts_buffer[i] != AV_NOPTS_VALUE) {
                    int64_t diff =  FFABS(pts_buffer[i] - dts)
                                    + (uint64_t)st->pts_reorder_error[i];
                    diff = FFMAX(diff, st->pts_reorder_error[i]);
                    st->pts_reorder_error[i] = diff;
                    st->pts_reorder_error_count[i]++;
                    if (st->pts_reorder_error_count[i] > 250) {
                        st->pts_reorder_error[i] >>= 1;
                        st->pts_reorder_error_count[i] >>= 1;
                    }
                }
            }
        }
    }

    if (dts == AV_NOPTS_VALUE)
        dts = pts_buffer[0];

    return dts;
}
static void update_dts_from_pts(AVFormatContext *s, int stream_index,
                                AVPacketList *pkt_buffer)
{
    AVStream *st       = s->streams[stream_index];
    int delay          = st->internal->avctx->has_b_frames;
    int i;

    int64_t pts_buffer[MAX_REORDER_DELAY+1];

    for (i = 0; i<MAX_REORDER_DELAY+1; i++)
        pts_buffer[i] = AV_NOPTS_VALUE;

    for (; pkt_buffer; pkt_buffer = get_next_pkt(s, st, pkt_buffer)) {
        if (pkt_buffer->pkt.stream_index != stream_index)
            continue;

        if (pkt_buffer->pkt.pts != AV_NOPTS_VALUE && delay <= MAX_REORDER_DELAY) {
            pts_buffer[0] = pkt_buffer->pkt.pts;
            for (i = 0; i<delay && pts_buffer[i] > pts_buffer[i + 1]; i++)
                FFSWAP(int64_t, pts_buffer[i], pts_buffer[i + 1]);

            pkt_buffer->pkt.dts = select_from_pts_buffer(st, pts_buffer, pkt_buffer->pkt.dts);
        }
    }
}
static void update_initial_timestamps(AVFormatContext *s, int stream_index,
                                      int64_t dts, int64_t pts, AVPacket *pkt)
{
    AVStream *st       = s->streams[stream_index];
    AVPacketList *pktl = s->internal->packet_buffer ? s->internal->packet_buffer : s->internal->parse_queue;
    AVPacketList *pktl_it;

    uint64_t shift;

    if (st->first_dts != AV_NOPTS_VALUE ||
        dts           == AV_NOPTS_VALUE ||
        st->cur_dts   == AV_NOPTS_VALUE ||
        st->cur_dts < INT_MIN + RELATIVE_TS_BASE ||
        is_relative(dts))
        return;

    st->first_dts = dts - (st->cur_dts - RELATIVE_TS_BASE);
    st->cur_dts   = dts;
    shift         = (uint64_t)st->first_dts - RELATIVE_TS_BASE;

    if (is_relative(pts))
        pts += shift;

    for (pktl_it = pktl; pktl_it; pktl_it = get_next_pkt(s, st, pktl_it)) {
        if (pktl_it->pkt.stream_index != stream_index)
            continue;
        if (is_relative(pktl_it->pkt.pts))
            pktl_it->pkt.pts += shift;

        if (is_relative(pktl_it->pkt.dts))
            pktl_it->pkt.dts += shift;

        if (st->start_time == AV_NOPTS_VALUE && pktl_it->pkt.pts != AV_NOPTS_VALUE) {
            st->start_time = pktl_it->pkt.pts;
            if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && st->codecpar->sample_rate)
                st->start_time += av_rescale_q(st->skip_samples, (AVRational){1, st->codecpar->sample_rate}, st->time_base);
        }
    }

    if (has_decode_delay_been_guessed(st)) {
        update_dts_from_pts(s, stream_index, pktl);
    }

    if (st->start_time == AV_NOPTS_VALUE) {
        if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO || !(pkt->flags & AV_PKT_FLAG_DISCARD)) {
            st->start_time = pts;
        }
        if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && st->codecpar->sample_rate)
            st->start_time += av_rescale_q(st->skip_samples, (AVRational){1, st->codecpar->sample_rate}, st->time_base);
    }
}
static void update_initial_durations(AVFormatContext *s, AVStream *st,
                                     int stream_index, int duration)
{
    AVPacketList *pktl = s->internal->packet_buffer ? s->internal->packet_buffer : s->internal->parse_queue;
    int64_t cur_dts    = RELATIVE_TS_BASE;

    if (st->first_dts != AV_NOPTS_VALUE) {
        if (st->update_initial_durations_done)
            return;
        st->update_initial_durations_done = 1;
        cur_dts = st->first_dts;
        for (; pktl; pktl = get_next_pkt(s, st, pktl)) {
            if (pktl->pkt.stream_index == stream_index) {
                if (pktl->pkt.pts != pktl->pkt.dts  ||
                    pktl->pkt.dts != AV_NOPTS_VALUE ||
                    pktl->pkt.duration)
                    break;
                cur_dts -= duration;
            }
        }
        if (pktl && pktl->pkt.dts != st->first_dts) {
            av_log(s, AV_LOG_DEBUG, "first_dts %s not matching first dts %s (pts %s, duration %"PRId64") in the queue\n",
                   av_ts2str(st->first_dts), av_ts2str(pktl->pkt.dts), av_ts2str(pktl->pkt.pts), pktl->pkt.duration);
            return;
        }
        if (!pktl) {
            av_log(s, AV_LOG_DEBUG, "first_dts %s but no packet with dts in the queue\n", av_ts2str(st->first_dts));
            return;
        }
        pktl          = s->internal->packet_buffer ? s->internal->packet_buffer : s->internal->parse_queue;
        st->first_dts = cur_dts;
    } else if (st->cur_dts != RELATIVE_TS_BASE)
        return;

    for (; pktl; pktl = get_next_pkt(s, st, pktl)) {
        if (pktl->pkt.stream_index != stream_index)
            continue;
        if ((pktl->pkt.pts == pktl->pkt.dts ||
             pktl->pkt.pts == AV_NOPTS_VALUE) &&
            (pktl->pkt.dts == AV_NOPTS_VALUE ||
             pktl->pkt.dts == st->first_dts ||
             pktl->pkt.dts == RELATIVE_TS_BASE) &&
            !pktl->pkt.duration) {
            pktl->pkt.dts = cur_dts;
            if (!st->internal->avctx->has_b_frames)
                pktl->pkt.pts = cur_dts;
//            if (st->codecpar->codec_type != AVMEDIA_TYPE_AUDIO)
            pktl->pkt.duration = duration;
        } else
            break;
        cur_dts = pktl->pkt.dts + pktl->pkt.duration;
    }
    if (!pktl)
        st->cur_dts = cur_dts;
}
static int is_intra_only(enum AVCodecID id)
{
    const AVCodecDescriptor *d = avcodec_descriptor_get(id);
    if (!d)
        return 0;
    if (d->type == AVMEDIA_TYPE_VIDEO && !(d->props & AV_CODEC_PROP_INTRA_ONLY))
        return 0;
    return 1;
}
// copy from ffmpeg n4.2.1
void av_compute_pkt_fields(AVFormatContext *s, AVStream *st,
                                      AVCodecParserContext *pc, AVPacket *pkt,
                                      int64_t next_dts, int64_t next_pts)
{
    int num, den, presentation_delayed, delay, i;
    int64_t offset;
    AVRational duration;
    int onein_oneout = st->codecpar->codec_id != AV_CODEC_ID_H264 &&
                       st->codecpar->codec_id != AV_CODEC_ID_HEVC;

    if (s->flags & AVFMT_FLAG_NOFILLIN)
        return;

    if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && pkt->dts != AV_NOPTS_VALUE) {
        if (pkt->dts == pkt->pts && st->last_dts_for_order_check != AV_NOPTS_VALUE) {
            if (st->last_dts_for_order_check <= pkt->dts) {
                st->dts_ordered++;
            } else {
                av_log(s, st->dts_misordered ? AV_LOG_DEBUG : AV_LOG_WARNING,
                       "DTS %"PRIi64" < %"PRIi64" out of order\n",
                       pkt->dts,
                       st->last_dts_for_order_check);
                st->dts_misordered++;
            }
            if (st->dts_ordered + st->dts_misordered > 250) {
                st->dts_ordered    >>= 1;
                st->dts_misordered >>= 1;
            }
        }

        st->last_dts_for_order_check = pkt->dts;
        if (st->dts_ordered < 8*st->dts_misordered && pkt->dts == pkt->pts)
            pkt->dts = AV_NOPTS_VALUE;
    }

    if ((s->flags & AVFMT_FLAG_IGNDTS) && pkt->pts != AV_NOPTS_VALUE)
        pkt->dts = AV_NOPTS_VALUE;

    if (pc && pc->pict_type == AV_PICTURE_TYPE_B
        && !st->internal->avctx->has_b_frames)
        //FIXME Set low_delay = 0 when has_b_frames = 1
        st->internal->avctx->has_b_frames = 1;

    /* do we have a video B-frame ? */
    delay = st->internal->avctx->has_b_frames;
    presentation_delayed = 0;

    /* XXX: need has_b_frame, but cannot get it if the codec is
     *  not initialized */
    if (delay &&
        pc && pc->pict_type != AV_PICTURE_TYPE_B)
        presentation_delayed = 1;

    if (pkt->pts != AV_NOPTS_VALUE && pkt->dts != AV_NOPTS_VALUE &&
        st->pts_wrap_bits < 63 &&
        pkt->dts - (1LL << (st->pts_wrap_bits - 1)) > pkt->pts) {
        if (is_relative(st->cur_dts) || pkt->dts - (1LL<<(st->pts_wrap_bits - 1)) > st->cur_dts) {
            pkt->dts -= 1LL << st->pts_wrap_bits;
        } else
            pkt->pts += 1LL << st->pts_wrap_bits;
    }

    /* Some MPEG-2 in MPEG-PS lack dts (issue #171 / input_file.mpg).
     * We take the conservative approach and discard both.
     * Note: If this is misbehaving for an H.264 file, then possibly
     * presentation_delayed is not set correctly. */
    if (delay == 1 && pkt->dts == pkt->pts &&
        pkt->dts != AV_NOPTS_VALUE && presentation_delayed) {
        av_log(s, AV_LOG_DEBUG, "invalid dts/pts combination %"PRIi64"\n", pkt->dts);
        if (    strcmp(s->iformat->name, "mov,mp4,m4a,3gp,3g2,mj2")
                && strcmp(s->iformat->name, "flv")) // otherwise we discard correct timestamps for vc1-wmapro.ism
            pkt->dts = AV_NOPTS_VALUE;
    }

    duration = av_mul_q((AVRational) {pkt->duration, 1}, st->time_base);
    if (pkt->duration == 0) {
        ff_compute_frame_duration(s, &num, &den, st, pc, pkt);
        if (den && num) {
            duration = (AVRational) {num, den};
            pkt->duration = av_rescale_rnd(1,
                                           num * (int64_t) st->time_base.den,
                                           den * (int64_t) st->time_base.num,
                                           AV_ROUND_DOWN);
        }
    }

    if (pkt->duration != 0 && (s->internal->packet_buffer || s->internal->parse_queue))
        update_initial_durations(s, st, pkt->stream_index, pkt->duration);

    /* Correct timestamps with byte offset if demuxers only have timestamps
     * on packet boundaries */
    if (pc && st->need_parsing == AVSTREAM_PARSE_TIMESTAMPS && pkt->size) {
        /* this will estimate bitrate based on this frame's duration and size */
        offset = av_rescale(pc->offset, pkt->duration, pkt->size);
        if (pkt->pts != AV_NOPTS_VALUE)
            pkt->pts += offset;
        if (pkt->dts != AV_NOPTS_VALUE)
            pkt->dts += offset;
    }

    /* This may be redundant, but it should not hurt. */
    if (pkt->dts != AV_NOPTS_VALUE &&
        pkt->pts != AV_NOPTS_VALUE &&
        pkt->pts > pkt->dts)
        presentation_delayed = 1;

    if (s->debug & FF_FDEBUG_TS)
        av_log(s, AV_LOG_DEBUG,
               "IN delayed:%d pts:%s, dts:%s cur_dts:%s st:%d pc:%p duration:%"PRId64" delay:%d onein_oneout:%d\n",
               presentation_delayed, av_ts2str(pkt->pts), av_ts2str(pkt->dts), av_ts2str(st->cur_dts),
               pkt->stream_index, pc, pkt->duration, delay, onein_oneout);

    /* Interpolate PTS and DTS if they are not present. We skip H264
     * currently because delay and has_b_frames are not reliably set. */
    if ((delay == 0 || (delay == 1 && pc)) &&
        onein_oneout) {
        if (presentation_delayed) {
            /* DTS = decompression timestamp */
            /* PTS = presentation timestamp */
            if (pkt->dts == AV_NOPTS_VALUE)
                pkt->dts = st->last_IP_pts;
            update_initial_timestamps(s, pkt->stream_index, pkt->dts, pkt->pts, pkt);
            if (pkt->dts == AV_NOPTS_VALUE)
                pkt->dts = st->cur_dts;

            /* This is tricky: the dts must be incremented by the duration
             * of the frame we are displaying, i.e. the last I- or P-frame. */
            if (st->last_IP_duration == 0 && (uint64_t)pkt->duration <= INT32_MAX)
                st->last_IP_duration = pkt->duration;
            if (pkt->dts != AV_NOPTS_VALUE)
                st->cur_dts = pkt->dts + st->last_IP_duration;
            if (pkt->dts != AV_NOPTS_VALUE &&
                pkt->pts == AV_NOPTS_VALUE &&
                st->last_IP_duration > 0 &&
                ((uint64_t)st->cur_dts - (uint64_t)next_dts + 1) <= 2 &&
                next_dts != next_pts &&
                next_pts != AV_NOPTS_VALUE)
                pkt->pts = next_dts;

            if ((uint64_t)pkt->duration <= INT32_MAX)
                st->last_IP_duration = pkt->duration;
            st->last_IP_pts      = pkt->pts;
            /* Cannot compute PTS if not present (we can compute it only
             * by knowing the future. */
        } else if (pkt->pts != AV_NOPTS_VALUE ||
                   pkt->dts != AV_NOPTS_VALUE ||
                   pkt->duration                ) {

            /* presentation is not delayed : PTS and DTS are the same */
            if (pkt->pts == AV_NOPTS_VALUE)
                pkt->pts = pkt->dts;
            update_initial_timestamps(s, pkt->stream_index, pkt->pts,
                                      pkt->pts, pkt);
            if (pkt->pts == AV_NOPTS_VALUE)
                pkt->pts = st->cur_dts;
            pkt->dts = pkt->pts;
            if (pkt->pts != AV_NOPTS_VALUE)
                st->cur_dts = av_add_stable(st->time_base, pkt->pts, duration, 1);
        }
    }

    if (pkt->pts != AV_NOPTS_VALUE && delay <= MAX_REORDER_DELAY) {
        st->pts_buffer[0] = pkt->pts;
        for (i = 0; i<delay && st->pts_buffer[i] > st->pts_buffer[i + 1]; i++)
            FFSWAP(int64_t, st->pts_buffer[i], st->pts_buffer[i + 1]);

        if(has_decode_delay_been_guessed(st))
            pkt->dts = select_from_pts_buffer(st, st->pts_buffer, pkt->dts);
    }
    // We skipped it above so we try here.
    if (!onein_oneout)
        // This should happen on the first packet
        update_initial_timestamps(s, pkt->stream_index, pkt->dts, pkt->pts, pkt);
    if (pkt->dts > st->cur_dts)
        st->cur_dts = pkt->dts;

    if (s->debug & FF_FDEBUG_TS)
        av_log(s, AV_LOG_DEBUG, "OUTdelayed:%d/%d pts:%s, dts:%s cur_dts:%s st:%d (%d)\n",
               presentation_delayed, delay, av_ts2str(pkt->pts), av_ts2str(pkt->dts), av_ts2str(st->cur_dts), st->index, st->id);

    /* update flags */
    if (st->codecpar->codec_type == AVMEDIA_TYPE_DATA || is_intra_only(st->codecpar->codec_id))
        pkt->flags |= AV_PKT_FLAG_KEY;
#if FF_API_CONVERGENCE_DURATION
    FF_DISABLE_DEPRECATION_WARNINGS
    if (pc)
        pkt->convergence_duration = pc->convergence_duration;
    FF_ENABLE_DEPRECATION_WARNINGS
#endif
}
