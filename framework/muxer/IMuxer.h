//
// Created by lifujun on 2019/3/1.
//

#ifndef SOURCE_IMUXER_H
#define SOURCE_IMUXER_H

#include <string>
#include <functional>
#include <utils/AFMediaType.h>
#include <base/media/IAFPacket.h>
#include <map>
#include <utils/mediaTypeInternal.h>

class IMuxer {
public:
    enum DataType {
        DATA_HEADER,
        DATA_SYNC_POINT,
        DATA_BOUNDARY_POINT,
        DATA_UNKNOWN,
        DATA_TRAILER,
        DATA_FLUSH_POINT,
    };
    typedef int64_t (*seekCallback)(void *opaque, int64_t offset, int whence);

    typedef int (*writePacketCallback)(void *opaque, uint8_t *buf, int size);

    typedef int (*writeDataTypeCallback)(void *opaque, uint8_t *buf, int size,
                                         IMuxer::DataType type, int64_t time);
    virtual ~IMuxer() = default;

    virtual void setCopyPts(bool copyPts) = 0;

    /**
     * add streamMetas
     * @param streamMetas
     */
    virtual void setStreamMetas(const std::vector<Stream_meta*> *streamMetas) = 0;

    //must be set before open(). These will be write to header.
    virtual void addSourceMetas(std::map<std::string,std::string> sourceMetas) = 0;

    virtual int open() = 0;

    virtual void setOpenFunc(std::function<void()> func) = 0;

    virtual int close() = 0;

    virtual void setCloseFunc(std::function<void()> func) = 0;

    virtual int muxPacket(std::unique_ptr<IAFPacket> packet) = 0;

    virtual void setWritePacketCallback(writePacketCallback callback, void *opaque) = 0;

    virtual void setSeekCallback(seekCallback callback, void *opaque) = 0;

    virtual void setWriteDataTypeCallback(writeDataTypeCallback callback, void *opaque) = 0;

};


#endif //SOURCE_IMUXER_H
