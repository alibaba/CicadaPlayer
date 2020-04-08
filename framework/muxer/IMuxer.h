//
// Created by lifujun on 2019/3/1.
//

#ifndef SOURCE_IAPASARAMUXER_H
#define SOURCE_IAPASARAMUXER_H

#include <string>
#include <functional>
#include <utils/AFMediaType.h>
#include <base/media/IAFPacket.h>
#include <map>
#include <utils/mediaTypeInternal.h>

using namespace std;

enum ApsaraDataType {
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
                                     enum ApsaraDataType type, int64_t time);

class IMuxer {

public:
    virtual ~IMuxer() = default;

    virtual void setCopyPts(bool copyPts) = 0;

    /**
     * add streamMetas
     * @param streamMetas
     */
    virtual void setStreamMetas(const vector<Stream_meta*> *streamMetas) = 0;

    //must be set before open(). These will be write to header.
    virtual void addSourceMetas(map<string,string> sourceMetas) = 0;

    virtual int open() = 0;

    virtual void setOpenFunc(function<void()> func) = 0;

    virtual int close() = 0;

    virtual void setCloseFunc(function<void()> func) = 0;

    virtual int muxPacket(unique_ptr<IAFPacket> packet) = 0;

    virtual void setWritePacketCallback(writePacketCallback callback, void *opaque) = 0;

    virtual void setSeekCallback(seekCallback callback, void *opaque) = 0;

    virtual void setWriteDataTypeCallback(writeDataTypeCallback callback, void *opaque) = 0;

};


#endif //SOURCE_IAPASARAMUXER_H
