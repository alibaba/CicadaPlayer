#include <utility>

//
// Created by lifujun on 2019/3/1.
//

#define LOG_TAG "FfmpegMuxer"

extern "C" {
#include <libavutil/opt.h>
};

#include <utils/frame_work_log.h>
#include <base/media/AVAFPacket.h>
#include <utils/ffmpeg_utils.h>
#include "FfmpegMuxer.h"
#include "MetaToCodec.h"
#include <utils/mediaFrame.h>


#define IO_BUFFER_SIZE 32768

using namespace std;
FfmpegMuxer FfmpegMuxer::se(0);

FfmpegMuxer::FfmpegMuxer(string destFilePath, string destFormat)
{
    mDestFilePath = std::move(destFilePath);
    mDestFormat = std::move(destFormat);
}


FfmpegMuxer::~FfmpegMuxer()
{
    if (mIobuf != nullptr) {
        free(mIobuf);
        mIobuf = nullptr;
    }

    mSourceMetaMap.clear();
    mStreamInfoMap.clear();
}

int FfmpegMuxer::open()
{
    ffmpeg_init();
    int ret = avformat_alloc_output_context2(&mDestFormatContext, nullptr, mDestFormat.c_str(),
              mDestFilePath.c_str());

    if (mDestFormatContext == nullptr) {
        AF_LOGE("Can't alloc_output_context ret = %d ,mDestFormat = %s , mDestFilePath = %s \n",
                ret, mDestFormat.c_str(), mDestFilePath.c_str() );
        return ret;
    }

    if(mStreamMetas != nullptr ) {
        for (Stream_meta *item : *mStreamMetas) {
            AVStream *stream = nullptr;

            if (item->type == Stream_type::STREAM_TYPE_VIDEO) {
                stream = avformat_new_stream(mDestFormatContext, nullptr);
                MetaToCodec::videoMetaToStream(stream, item);
                check_codec_tag(stream);
            } else if (item->type == Stream_type::STREAM_TYPE_AUDIO) {
                stream = avformat_new_stream(mDestFormatContext, nullptr);
                MetaToCodec::audioMetaToStream(stream, item);
                check_codec_tag(stream);
            }

            insertStreamInfo(stream, item);
        }
    }

    if (mOpenFunc != nullptr) {
        mOpenFunc();
    }

    mIobuf = (uint8_t *) malloc(IO_BUFFER_SIZE);
    mDestFormatContext->pb = avio_alloc_context(mIobuf, IO_BUFFER_SIZE,
                             AVIO_FLAG_WRITE, this,
                             nullptr, io_write, io_seek);
    mDestFormatContext->pb->write_data_type = io_write_data_type;

    if (!mSourceMetaMap.empty()) {
        map<string, string>::iterator mapIt;

        for (mapIt = mSourceMetaMap.begin(); mapIt != mSourceMetaMap.end(); mapIt++) {
            av_dict_set(&mDestFormatContext->metadata, mapIt->first.c_str(), mapIt->second.c_str(),
                        0);
        }
    }

    ret = avformat_write_header(mDestFormatContext, nullptr);

    if (ret < 0) {
        AF_LOGE(" write header fail: ret = %d , to output file '%s'", ret, mDestFilePath.c_str());
        return ret;
    }

    return 0;
}

void FfmpegMuxer::check_codec_tag(const AVStream *stream)
{
    if (stream->codecpar->codec_tag && mDestFormatContext->oformat->codec_tag) {
        if (av_codec_get_id(mDestFormatContext->oformat->codec_tag, stream->codecpar->codec_tag) !=
                stream->codecpar->codec_id) {
            const uint32_t otag = av_codec_get_tag(mDestFormatContext->oformat->codec_tag,
                                                   stream->codecpar->codec_id);
#ifndef _WIN32
            AF_LOGW("Tag %s incompatible with output codec id '%d' (%s)\n",
                    av_fourcc2str(stream->codecpar->codec_tag), stream->codecpar->codec_id, av_fourcc2str(otag));
#endif
            stream->codecpar->codec_tag = otag;
        }
    }

    /*
     * quick time player check hevc tag is hvc1 in mp4
     */
    if (stream->codecpar->codec_id == AV_CODEC_ID_HEVC &&
            strcmp(mDestFormatContext->oformat->name, "mp4") == 0) {
        stream->codecpar->codec_tag = MKTAG('h', 'v', 'c', '1');
    }
}


void FfmpegMuxer::insertStreamInfo(const AVStream *st, const Stream_meta *meta)
{
    if (st == nullptr || meta == nullptr) {
        return;
    }

    AVOutputFormat *fmt = mDestFormatContext->oformat;
    AVRational timeBase{};

    if (!strncmp(fmt->name, "flv", 3)) {
        timeBase = {1, 1000};
    } else if (!strncmp(fmt->name, "mpegts", 6)) {
        timeBase = {1, 90000};
    } else {
        timeBase = st->time_base;
    }

    StreamInfo streamInfo{};
    streamInfo.timeBase = timeBase;
    streamInfo.targetIndex = st->index;
    mStreamInfoMap.insert(pair<int, StreamInfo>(meta->index, streamInfo));
}

int FfmpegMuxer::writeFrame(unique_ptr<IAFPacket> packetPtr)
{
    if (mDestFormatContext == nullptr) {
        AF_LOGE("mDestFormatContext is null..");
        return -1;
    }

    AVPacket *avPacket = getAVPacket(packetPtr.get());

    if (avPacket == nullptr) {
        AF_LOGE("muxer packet is null..");
        return -1;
    }

    int pktStreamIndex = avPacket->stream_index;
    StreamInfo &streamInfo = mStreamInfoMap[pktStreamIndex];

    if (mStreamInfoMap.count(pktStreamIndex) == 0) {
        AF_LOGE("no such index %d", pktStreamIndex);
        return -1;
    }

    AVPacket *pkt = av_packet_clone(avPacket);

    if (mFirstPts == INT64_MIN) {
        mFirstPts = avPacket->pts;
    }

    pkt->stream_index = streamInfo.targetIndex;

    if (!bCopyPts) {
        if (mFirstPts != INT64_MIN) {
            if (pkt->pts != INT64_MIN) {
                pkt->pts -= mFirstPts;
            }

            if (pkt->dts != INT64_MIN) {
                pkt->dts -= mFirstPts;
            }
        }
    }

    AVRational timeBase = streamInfo.timeBase;
    pkt->pts = av_rescale_q(pkt->pts, {1, AV_TIME_BASE}, timeBase);
    pkt->dts = av_rescale_q(pkt->dts, {1, AV_TIME_BASE}, timeBase);
    {
        //make sure dts increasing
        int64_t dts = streamInfo.lastDts;

        if (dts != INT64_MAX) {
            if (pkt->dts <= dts) {
                pkt->dts = dts + 1;
            }
        }
    }
    streamInfo.lastDts = pkt->dts;

    if (pkt->pts < pkt->dts) {
        pkt->pts = pkt->dts;
    }

    mDestFormatContext->max_interleave_delta = 0;
    int ret = av_interleaved_write_frame(mDestFormatContext, pkt);
    av_packet_free(&pkt);

    if (ret < 0) {
        AF_LOGE("write packet failed . ret = %d. pktStreamIndex index = %d , stream index = %d ", ret, pktStreamIndex, streamInfo.targetIndex);
        return ret;
    }

    return 0;
}

int FfmpegMuxer::muxPacket(unique_ptr<IAFPacket> packet)
{
    return writeFrame(move(packet));
}

int FfmpegMuxer::close()
{
    if (mDestFormatContext == nullptr) {
        return 0;
    }

    int ret = 0;
    ret = av_write_trailer(mDestFormatContext);

    if (ret < 0) {
        AF_LOGE("av_write_trailer fail ret is %d\n", ret);
    }

    if (mDestFormatContext->metadata) {
        av_dict_free(&mDestFormatContext->metadata);
    }

    avio_flush(mDestFormatContext->pb);
    av_opt_free(mDestFormatContext->pb);
    av_free(mDestFormatContext->pb);
    avformat_free_context(mDestFormatContext);
    mDestFormatContext = nullptr;

    if (mCloseFunc != nullptr) {
        mCloseFunc();
    }

    mFirstPts = INT64_MIN;
    return ret;
}

void FfmpegMuxer::setWritePacketCallback(writePacketCallback callback, void *opaque)
{
    mWritePacketCallback = callback;
    mWritePacketOpaque = opaque;
}

void FfmpegMuxer::setSeekCallback(seekCallback callback, void *opaque)
{
    mSeekCallback = callback;
    mSeekOpaque = opaque;
}

void FfmpegMuxer::setWriteDataTypeCallback(writeDataTypeCallback callback, void *opaque)
{
    mWriteDataTypeCallback = callback;
    mWriteDataTypeOpaque = opaque;
}

int FfmpegMuxer::io_write(void *opaque, uint8_t *buf, int size)
{
    auto *ffmpegMux = static_cast<FfmpegMuxer *>(opaque);
    return ffmpegMux->muxerWrite(buf, size);
}

int64_t FfmpegMuxer::io_seek(void *opaque, int64_t offset, int whence)
{
    auto *ffmpegMux = static_cast<FfmpegMuxer *>(opaque);
    return ffmpegMux->muxerSeek(offset, whence);
}

int FfmpegMuxer::io_write_data_type(void *opaque, uint8_t *buf, int size, enum AVIODataMarkerType type,
                                    int64_t time)
{
    auto *ffmpegMux = static_cast<FfmpegMuxer *>(opaque);
    DataType dataType = ffmpegMux->mapType(type);
    return ffmpegMux->muxerWriteDataType(buf, size, dataType, time);
}

IMuxer::DataType FfmpegMuxer::mapType(AVIODataMarkerType type)
{
    if (type == AVIO_DATA_MARKER_SYNC_POINT) {
        return DATA_SYNC_POINT;
    }

    if (type == AVIO_DATA_MARKER_BOUNDARY_POINT) {
        return DATA_BOUNDARY_POINT;
    }

    if (type == AVIO_DATA_MARKER_FLUSH_POINT) {
        return DATA_FLUSH_POINT;
    }

    if (type == AVIO_DATA_MARKER_HEADER) {
        return DATA_HEADER;
    }

    if (type == AVIO_DATA_MARKER_TRAILER) {
        return DATA_TRAILER;
    }

    if (type == AVIO_DATA_MARKER_UNKNOWN) {
        return DATA_UNKNOWN;
    }

    return DATA_UNKNOWN;
}

void FfmpegMuxer::setOpenFunc(function<void()> func)
{
    mOpenFunc = func;
}

void FfmpegMuxer::setCloseFunc(function<void()> func)
{
    mCloseFunc = func;
}

void FfmpegMuxer::setCopyPts(bool copyPts)
{
    bCopyPts = copyPts;
}

int64_t FfmpegMuxer::muxerSeek(int64_t offset, int whence)
{
    if (mSeekCallback != nullptr) {
        return mSeekCallback(mSeekOpaque, offset, whence);
    } else {
        return offset;
    }
}

int FfmpegMuxer::muxerWrite(uint8_t *buf, int size)
{
    if (mWritePacketCallback != nullptr) {
        return mWritePacketCallback(mWritePacketOpaque, buf, size);
    } else {
        return size;
    }
}

int FfmpegMuxer::muxerWriteDataType(uint8_t *buf, int size, enum DataType type,
                                    int64_t time)
{
    if (mWriteDataTypeCallback != nullptr) {
        return mWriteDataTypeCallback(mWriteDataTypeOpaque, buf, size, type, time);
    } else {
        return size;
    }
}

void FfmpegMuxer::setStreamMetas(const vector<Stream_meta *> *streamMetas)
{
    mStreamMetas = streamMetas;
}

void FfmpegMuxer::addSourceMetas(map<string, string> sourceMetas)
{
    if (sourceMetas.empty()) {
        return;
    }

    for (auto &sourceMeta : sourceMetas) {
        mSourceMetaMap.insert(pair<string, string>(sourceMeta.first, sourceMeta.second));
    }
}
