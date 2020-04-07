//
// Created by moqi on 2019-07-08.
//

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avc.h>
#include <libavformat/hevc.h>
#include <libavformat/avio_internal.h>
#include <pthread.h>
#include <utils/frame_work_log.h>
#include <libavutil/avstring.h>
#include <assert.h>
#include "ffmpeg_utils.h"

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
//        case AV_CODEC_ID_TEXT:
//            return AF_CODEC_ID_TEXT;
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
    {AF_PIX_FMT_NONE,      AV_PIX_FMT_NONE},
    {AF_PIX_FMT_YUV420P,   AV_PIX_FMT_YUV420P},
    {AF_PIX_FMT_YUV422P,   AV_PIX_FMT_YUV422P},
    {AF_PIX_FMT_YUVJ420P,  AV_PIX_FMT_YUVJ420P},
    {AF_PIX_FMT_YUVJ422P,  AV_PIX_FMT_YUVJ422P},
    {AF_PIX_FMT_D3D11,     AV_PIX_FMT_D3D11},
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

int get_stream_meta(struct AVStream *pStream, Stream_meta *meta)
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

    if (codec_type == AVMEDIA_TYPE_VIDEO) {
        if (pStream->codecpar->sample_aspect_ratio.num) {
            meta->displayWidth = pStream->codecpar->width;
            meta->displayWidth *= pStream->codecpar->sample_aspect_ratio.num;
            meta->displayWidth /= pStream->codecpar->sample_aspect_ratio.den;
            meta->displayHeight = pStream->codecpar->height;
//           av_log(NULL, AV_LOG_ERROR, "DAR %d:%d", meta->displayWidth, meta->displayHeight);
        } else {
            meta->displayWidth = meta->displayHeight = 0;
        }

        meta->type = STREAM_TYPE_VIDEO;
        meta->width = pStream->codecpar->width;
        meta->height = pStream->codecpar->height;
        meta->profile = pStream->codecpar->profile;
        meta->pixel_fmt = pStream->codecpar->format;

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
        memcpy(buffer, frame->extended_data[0], (size_t) (sampleSize * frame->nb_samples * frame->channels));
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
        if (outSize > (sampleSize * frame->nb_samples * frame->channels - frameOffset)) {
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
            fillCallback(args, frame->extended_data[0], (size_t) (sampleSize * frame->nb_samples * frame->channels));
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
