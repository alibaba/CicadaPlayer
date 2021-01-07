//
// Created by moqi on 2019-07-08.
//

#ifndef FRAMEWORK_FFMPEG_UTILS_H
#define FRAMEWORK_FFMPEG_UTILS_H

#include <utils/AFMediaType.h>

#ifdef __cplusplus
extern "C"{
#endif

#include <libavutil/dict.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <stdbool.h>

#ifdef __cplusplus
}
#endif

typedef void(*fillBufferCallback)(void *args, uint8_t *src, size_t size);

#ifdef __cplusplus
extern "C"{
#endif

struct AVStream;

void ffmpeg_init();

void ffmpeg_deinit();

int AVDictionary2SourceMeta(Source_meta **meta, const AVDictionary *dictionary);

enum AFCodecID AVCodec2CicadaCodec(enum AVCodecID codec);

enum AVCodecID CodecID2AVCodecID(enum AFCodecID codec);


int set_stream_meta(struct AVStream *pStream, Stream_meta *meta);

int get_stream_meta(const struct AVStream *pStream, Stream_meta *meta);

int AVPixFmt2Cicada(enum AVPixelFormat fmt);

int AVColorSpace2AF(enum AVColorSpace space);

int AVColorRange2AF(enum AVColorRange range);

int af_get_bytes_per_sample(enum AFSampleFormat fmt);

void copyYuvData_yuv420p(const AVFrame *frame, unsigned char *buffer, int width, int height);

int getPCMDataLen(int channels, enum AVSampleFormat format, int nb_samples);

int getPCMFrameLen(const AVFrame *frame);

int getPCMFrameDuration(const AVFrame *frame);

void copyPCMData(const AVFrame *frame, uint8_t *buffer);

// read frame from frameOffset, and return copy size.
// frameClear will set to true if all the data in the frame be copyed
size_t copyPCMDataWithOffset(const AVFrame *frame, int frameOffset, uint8_t *outBuffer, size_t size, bool *frameClear);

void copyPCMData2(const AVFrame *frame, fillBufferCallback fillCallback, void *args);

bool updateH26xHeader2xxc(AVCodecParameters *par);

const char *getErrorString(int err);

int parse_h264_extraData(enum AVCodecID codecId, const uint8_t* extraData,int extraData_size,
                         uint8_t** sps_data ,int*sps_data_size,
                         uint8_t** pps_data, int* pps_data_size,
                         int* nal_length_size);

int parse_h265_extraData(enum AVCodecID codecId, const uint8_t* extradata,int extradata_size,
                         uint8_t** vps_data ,int*vps_data_size,
                         uint8_t** sps_data ,int*sps_data_size,
                         uint8_t** pps_data, int* pps_data_size,
                         int* nal_length_size);

void av_compute_pkt_fields(AVFormatContext *s, AVStream *st,
        AVCodecParserContext *pc, AVPacket *pkt,
        int64_t next_dts, int64_t next_pts);

#ifdef __cplusplus
};
#endif

#endif //FRAMEWORK_FFMPEG_UTILS_H
