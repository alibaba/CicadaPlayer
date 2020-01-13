//
// Created by lifujun on 2019/5/27.
//

#ifndef SOURCE_CACHEFILEREMUXER_H
#define SOURCE_CACHEFILEREMUXER_H

#include <string>
#include <queue>
#include <map>
#include <mutex>
#include <native_cicada_player_def.h>
#include <utils/AFMediaType.h>

#include <utils/afThread.h>
#include <muxer/IMuxer.h>

#include <utils/file/FileCntl.h>


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

    void setMetaCallback(function<bool(StreamType, Stream_meta *)> metaCallback);

    void addFrame(const unique_ptr<IAFPacket>& frame, StreamType type);

    bool prepare();

    void start();

    void stop();

    void interrupt();

    void setErrorCallback(function<void(int, string)> callback);

private :

    void initMuxer();

    int muxThreadRun();

    static int64_t io_seek(void *opaque, int64_t offset, int whence);

    static int io_write(void *opaque, uint8_t *buf, int size);

    static int io_write_data_type(void *opaque, uint8_t *buf, int size,
                                  enum ApsaraDataType type, int64_t time);


private:
    string mDestFilePath;
    string mDescription;
    function<bool(StreamType, Stream_meta *)> mMetaCallback = nullptr;
    deque<std::unique_ptr<FrameInfo>> mFrameInfoQueue;
    condition_variable mQueueCondition;

    bool mInterrupt = false;
    bool mWantStop = false;

    mutex mThreadMutex;
    mutex mObjectMutex;
    mutex mQueueMutex;

    afThread *mMuxThread = nullptr;
    IMuxer *mMuxer = nullptr;
    FileCntl *mDestFileCntl = nullptr;

    function<void(int, string)> mErrorCallback = nullptr;

};


#endif //SOURCE_CACHEFILEREMUXER_H
