//
// Created by lifujun on 2019/5/27.
//

#ifndef SOURCE_CACHEFILEREMUXER_H
#define SOURCE_CACHEFILEREMUXER_H

#include <string>
#include <queue>
#include <map>
#include <mutex>
#include <vector>
#include <native_cicada_player_def.h>
#include <utils/AFMediaType.h>

#include <utils/afThread.h>
#include <muxer/IMuxer.h>

#include <utils/file/FileCntl.h>
#include <utils/mediaTypeInternal.h>
#include "CacheRet.h"


using namespace std;

class FrameInfo {
public:
    std::unique_ptr<IAFPacket> frame;
    StreamType type = ST_TYPE_UNKNOWN;
};

class CacheFileRemuxer {

public:
    CacheFileRemuxer(const string &destFilePath, const string &description);

    ~CacheFileRemuxer();

    void addFrame(const unique_ptr<IAFPacket> &frame, StreamType type);

    bool prepare();

    void start();

    void stop();

    void interrupt();

    void setErrorCallback(function<void(int, string)> callback);

    void setResultCallback(function<void(bool)> callback);

    void setStreamMeta(const vector<Stream_meta *> *streamMetas);

private :

    void sendError(const CacheRet& ret);

    void initMuxer();

    int muxThreadRun();

    static int64_t io_seek(void *opaque, int64_t offset, int whence);

    static int io_write(void *opaque, uint8_t *buf, int size);

    static int io_write_data_type(void *opaque, uint8_t *buf, int size,
                                  IMuxer::DataType type, int64_t time);


private:
    string mDestFilePath;
    string mDescription;
    deque<std::unique_ptr<FrameInfo>> mFrameInfoQueue;
    condition_variable mQueueCondition;

    std::atomic_bool mInterrupt{false};
    std::atomic_bool mWantStop {false};
    std::atomic_bool mFrameEof{false};
    bool mRemuxSuc = true;


    mutex mThreadMutex;
    mutex mObjectMutex;
    mutex mQueueMutex;

    afThread *mMuxThread = nullptr;
    IMuxer *mMuxer = nullptr;
    FileCntl *mDestFileCntl = nullptr;

    function<void(int, string)> mErrorCallback = nullptr;
    function<void(bool)> mResultCallback = nullptr;

    const vector<Stream_meta*> *mStreamMetas = nullptr;

};


#endif //SOURCE_CACHEFILEREMUXER_H
