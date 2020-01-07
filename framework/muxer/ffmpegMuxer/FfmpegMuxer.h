//
// Created by lifujun on 2019/3/1.
//

#ifndef SOURCE_FFMPEGMUX_H
#define SOURCE_FFMPEGMUX_H

#include <string>
#include <functional>
#include <map>
#include <muxer/IMuxer.h>
#include "utils/CicadaType.h"

extern "C" {
#include <libavutil/rational.h>
#include <libavformat/avformat.h>
};

using namespace std;

#include <muxer/IMuxerPrototype.h>


class CICADA_CPLUS_EXTERN FfmpegMuxer : public IMuxer , private  IMuxerPrototype{
public:
    FfmpegMuxer(string destFilePath, string destFormat);

    ~FfmpegMuxer() override;

public:

    void setCopyPts(bool copyPts) override;

    void setGetVideoMetaCallback(function<bool(Stream_meta *)> function) override;

    void setGetAudioMetaCallback(function<bool(Stream_meta *)> function) override;

    void setWritePacketCallback(writePacketCallback callback, void *opaque) override;

    void setSeekCallback(seekCallback callback, void *opaque) override;

    void setWriteDataTypeCallback(writeDataTypeCallback callback, void *opaque) override;

    void setOpenFunc(function<void()> func) override;

    void setCloseFunc(function<void()> func) override;

    //must be set before open(). These will be write to header.
    void setMeta(string key, string value) override;

    void clearMeta() override;

    int open() override;

    int muxAudio(unique_ptr<IAFPacket> audioFrame) override;

    int muxVideo(unique_ptr<IAFPacket> videoFrame) override;

    int close() override;

protected:
    explicit FfmpegMuxer(int dummy){
        addPrototype(this);
    }

private:
    IMuxer *clone(const string& destPath , const string& destFormat, const string& description) override
    {
        return new FfmpegMuxer(destPath, destFormat);
    }

    bool is_supported(const string& destPath , const string& destFormat, const string& description) override
    {
        return true;
    }

    static FfmpegMuxer se;

protected:
    virtual int64_t muxerSeek(int64_t offset, int whence);

    virtual int muxerWrite(uint8_t *buf, int size);

    virtual int muxerWriteDataType(uint8_t *buf, int size,
                           enum ApsaraDataType type, int64_t time);

private:

    void fillAudioStreamInfo(AVStream *st, Stream_meta *meta);

    void fillVideoStreamInfo(AVStream *st, Stream_meta *meta);

    ApsaraDataType mapType(AVIODataMarkerType type);

    static int64_t io_seek(void *opaque, int64_t offset, int whence);

    static int io_write(void *opaque, uint8_t *buf, int size);

    static int io_write_data_type(void *opaque, uint8_t *buf, int size,
                                  enum AVIODataMarkerType type, int64_t time);

    int writeFrame(unique_ptr<IAFPacket> framePtr, int index);
protected:

    AVFormatContext *mDestFormatContext = nullptr;

private:
    function<bool(Stream_meta *)> mGetVideoMetaCallback = nullptr;
    function<bool(Stream_meta *)> mGetAudioMetaCallback = nullptr;


    int mAudioStreamIndex = -1;
    int mVideoStreamIndex = -1;

    AVRational mConAbase{};
    AVRational mConVbase{};

    map<string, string> metaMap;

    uint8_t               *mIobuf = nullptr;
    seekCallback          mSeekCallback = nullptr;
    void                  *mSeekOpaque = nullptr;
    writePacketCallback   mWritePacketCallback = nullptr;
    void                  *mWritePacketOpaque = nullptr;
    writeDataTypeCallback mWriteDataTypeCallback = nullptr;
    void                  *mWriteDataTypeOpaque = nullptr;

    function<void()> mOpenFunc = nullptr;
    function<void()> mCloseFunc = nullptr;
    int64_t mFirstPts = INT64_MIN;
    int64_t mLastAudioDts = INT64_MAX;
    int64_t mLastVideoDts = INT64_MAX;
protected:
    string mDestFilePath;
    string mDestFormat;

    bool bCopyPts = false;

    void check_codec_tag(const AVStream *stream);
};


#endif //SOURCE_FFMPEGMUX_H
