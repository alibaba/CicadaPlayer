//
// Created by lifujun on 2019/3/1.
//

#ifndef SOURCE_FFMPEGMUX_H
#define SOURCE_FFMPEGMUX_H

#include <vector>
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


class CICADA_CPLUS_EXTERN FfmpegMuxer : public IMuxer, private IMuxerPrototype {
public:
    FfmpegMuxer(string destFilePath, string destFormat);

    virtual ~FfmpegMuxer() override;

public:

    void setCopyPts(bool copyPts) override;


    void setStreamMetas(const vector<Stream_meta*> &streamMetas) override;


    void clearStreamMetas() override ;

    //must be set before open(). These will be write to header.
    void addSourceMetas(map<string, string> sourceMetas) override;

    void setWritePacketCallback(writePacketCallback callback, void *opaque) override;

    void setSeekCallback(seekCallback callback, void *opaque) override;

    void setWriteDataTypeCallback(writeDataTypeCallback callback, void *opaque) override;

    void setOpenFunc(function<void()> func) override;

    void setCloseFunc(function<void()> func) override;

    int open() override;

    int muxPacket(unique_ptr<IAFPacket> packet) override;

    int close() override;

protected:
    explicit FfmpegMuxer(int dummy) {
        addPrototype(this);
    }

private:
    IMuxer *
    clone(const string &destPath, const string &destFormat, const string &description) override {
        return new FfmpegMuxer(destPath, destFormat);
    }

    int probeScore(const string &destPath, const string &destFormat,
                   const string &description) override {
        if(is_supported(destPath , destFormat,description)){
            return Cicada::SUPPORT_DEFAULT;
        }
        return 0;
    }

    bool is_supported(const string &destPath, const string &destFormat,
                      const string &description)  {
        return true;
    }

    static FfmpegMuxer se;

private:
    class StreamInfo {
    public:
        int targetIndex = -1;
        AVRational timeBase{};
        int64_t lastDts = INT64_MAX;
    };

protected:
    virtual int64_t muxerSeek(int64_t offset, int whence);

    virtual int muxerWrite(uint8_t *buf, int size);

    virtual int muxerWriteDataType(uint8_t *buf, int size,
                                   enum ApsaraDataType type, int64_t time);

private:

    void insertStreamInfo(const AVStream *st, const Stream_meta *meta);

    ApsaraDataType mapType(AVIODataMarkerType type);

    static int64_t io_seek(void *opaque, int64_t offset, int whence);

    static int io_write(void *opaque, uint8_t *buf, int size);

    static int io_write_data_type(void *opaque, uint8_t *buf, int size,
                                  enum AVIODataMarkerType type, int64_t time);

    int writeFrame(unique_ptr<IAFPacket> packetPtr);

    void check_codec_tag(const AVStream *stream);

protected:

    AVFormatContext *mDestFormatContext = nullptr;

private:

    map<string, string> mSourceMetaMap;
    vector<Stream_meta*> mStreamMetas;
    map<int, StreamInfo> mStreamInfoMap;

    uint8_t *mIobuf = nullptr;
    seekCallback mSeekCallback = nullptr;
    void *mSeekOpaque = nullptr;
    writePacketCallback mWritePacketCallback = nullptr;
    void *mWritePacketOpaque = nullptr;
    writeDataTypeCallback mWriteDataTypeCallback = nullptr;
    void *mWriteDataTypeOpaque = nullptr;

    function<void()> mOpenFunc = nullptr;
    function<void()> mCloseFunc = nullptr;
    int64_t mFirstPts = INT64_MIN;

protected:
    string mDestFilePath;
    string mDestFormat;

    bool bCopyPts = false;

};


#endif //SOURCE_FFMPEGMUX_H
