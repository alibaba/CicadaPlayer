//
// Created by moqi on 2019-07-19.
//
#define LOG_TAG "BSDecoder"
#include "WABitStreamDecoder.h"
#include "bitStreamDecoder.h"

#ifdef __EMSCRIPTEN__

    #include <emscripten.h>

#else

    #include <utils/afThread.h>

#endif

#include <memory>
#include <utils/timer.h>
#include <utils/ffmpeg_utils.h>

using namespace Cicada;

static std::unique_ptr<bitStreamDecoder> decoder{};
static data_callback_read callbackRead{};
static eos_call_back eosCallBack{};
static getBuffer_call_back bufferCallBack{};
static frame_call_back frameCallBack{};

static video_info_call_back videoInfoCallBack{};
static audio_info_call_back audioInfoCallBack{};
static poll_data pollDataCallBack{};
static error_call_back errorCallBack{};
static poll_raw_buffer pollRawBuffer{};

static bool readEos{};

static void pull_once();

static bool pollData();


enum decoderStatus_t {
    status_idle,
    status_prepared,
    status_error,
    status_eos,
};

struct metaData_t {
    int width = 0;
    int height = 0;
    int channels = 0;
    int sample_rate = 0;

    int duration = 0;
};

static metaData_t metaData;

static decoderStatus_t status = status_idle;

#ifdef __EMSCRIPTEN__

static int emscripten_read(void *arg, uint8_t *buffer, int size)
{
    int ret;

    do {
        ret = callbackRead(arg, buffer, size);
    } while (ret == -EAGAIN);

    AF_LOGD("ret is %d\n", ret);
    assert(ret <= size);
    return ret;
}

#endif

class WBListener : public bitStreamDecoder::eventListener
{
public:
    void onError(int error) override
    {
    };

    void onEOS(enum bitStreamDecoder::eosType type) override
    {
        AF_LOGD("EOS");

        if (type == bitStreamDecoder::eosTypeDemuxer) {
            readEos = true;
            return;
        }

        if (eosCallBack) {
            eosCallBack();
        }

        status = status_eos;
    };

    void onFrame(IAFFrame *frame, int consumeTime) override
    {
        //  frame->dump();
        if (frameCallBack == nullptr) {
            return;
        }

        IAFFrame::FrameType type = frame->getType();
        int size = 0;
        int pts = INT32_MIN;

        if (frame->getInfo().pts != INT64_MIN) {
            pts = frame->getInfo().pts / 1000;
        }

        switch (type) {
            case IAFFrame::FrameTypeVideo:
                // TODO: not yuv 420
                size = frame->getInfo().video.height * frame->getInfo().video.width * 3 / 2;
                break;

            case IAFFrame::FrameTypeAudio:
                size = frame->getInfo().audio.channels *
                       af_get_bytes_per_sample(static_cast<AFSampleFormat>(frame->getInfo().format))
                       * frame->getInfo().audio.nb_samples;
                break;

            default:
                AF_LOGE("UnKnown type\n");
                return;
        }

        uint8_t *buffer{};

        if (bufferCallBack) {
            buffer = bufferCallBack(size);

            if (buffer == nullptr) {
                return;
            }
        }

        if (buffer == nullptr) {
            buffer = static_cast<uint8_t *>(malloc(size));
        }

        if (type == IAFFrame::FrameTypeVideo) {
            copyYuvData_yuv420p(getAVFrame(frame), buffer, frame->getInfo().video.width,
                                frame->getInfo().video.height);

            if (metaData.height != frame->getInfo().video.height ||
                    metaData.width != frame->getInfo().video.width) {
                metaData.height = frame->getInfo().video.height;
                metaData.width = frame->getInfo().video.width;

                if (videoInfoCallBack) {
                    videoInfoCallBack(metaData.width, metaData.height, metaData.duration);
                }
            }

            frameCallBack(buffer, size, pts, frameTypeVideo, consumeTime);
        } else {
            copyPCMData(getAVFrame(frame), buffer);

            if (metaData.channels != frame->getInfo().audio.channels ||
                    metaData.sample_rate != frame->getInfo().audio.sample_rate) {
                metaData.channels = frame->getInfo().audio.channels;
                metaData.sample_rate = frame->getInfo().audio.sample_rate;

                if (audioInfoCallBack) {
                    audioInfoCallBack(metaData.channels, metaData.sample_rate, metaData.duration);
                }
            }

            frameCallBack(buffer, size, pts, frameTypeAudio, consumeTime);
        }

        if (!bufferCallBack) {
            free(buffer);
        }
    };

    void onStreamMeta(const Stream_meta *meta) override
    {
        if (meta->type == STREAM_TYPE_VIDEO) {
            metaData.height = meta->height;
            metaData.width = meta->width;
            metaData.duration = meta->duration / 1000;
            AF_LOGD("duration is %lld\n", meta->duration);

            if (videoInfoCallBack) {
                videoInfoCallBack(metaData.width, metaData.height, metaData.duration);
            }
        } else if (meta->type == STREAM_TYPE_AUDIO) {
            metaData.channels = meta->channels;
            metaData.sample_rate = meta->samplerate;
            metaData.duration = meta->duration / 1000;

            if (audioInfoCallBack) {
                audioInfoCallBack(metaData.channels, metaData.sample_rate, metaData.duration);
            }
        }
    }

    bool onPollData() override
    {
        return pollData();
    };


};

static WBListener listener{};

#ifndef __EMSCRIPTEN__
static std::unique_ptr<afThread> pThread(new afThread([]()
{
    pull_once();
    return 0;
}, "xxx"));
#endif

static const int POLL_SIZE = 1024 * 1024;

static bool pollData()
{
    int size = POLL_SIZE;

    if (status == status_idle) {
        size *= 2;
    }

    if (pollDataCallBack) {
        int ret = pollDataCallBack(nullptr);
        AF_LOGD("poll data size is %d\n", ret);

        if (ret == -EAGAIN || (ret < size && ret > 0)) {
            return false;
        }
    }

    return true;
}

class timerSetter
{
public:
    timerSetter(int ms = 10) : mTime(ms)
    {
    };


    ~timerSetter()
    {
#ifdef __EMSCRIPTEN__
        emscripten_set_main_loop_timing(EM_TIMING_SETTIMEOUT, mTime);
#endif
    }

    int operator=(int ms)
    {
        return mTime = ms;
    }

private:
    int mTime{10};

};

static void pull_once()
{
    timerSetter timer;

    if (status == status_idle) {
        if (!pollData()) {
            timer = 1000;
            return;
        }

        int ret = decoder->prepare();

        if (ret < 0) {
            status = status_error;

            if (errorCallBack) {
                errorCallBack(ret);
            }
        } else {
            status = status_prepared;
        }
    }

    if (status == status_prepared) {
        if (!readEos && decoder->getCacheDuration() < 1000000 * 10) {
            int count = 0;

            while (pollData()) {
                decoder->readPacket();

                if (count++ > 0) {
                    break;
                }
            }
        }

//        if (decoder->getCacheDuration() == 0)
//            timer = 1000;
        int count = 0;

        while (pollRawBuffer == nullptr || pollRawBuffer(nullptr, 0) == 0) {
            decoder->pull_once();

            if (count++ > 0) {
                break;
            }
        }
    }
}

#ifdef __EMSCRIPTEN__

void BSDecoderSetReadCallBack(long func)
{
    callbackRead = (data_callback_read) func;
}

void BSDecoderEOSCallBack(long func)
{
    eosCallBack = (eos_call_back) func;
}

void BSDecoderSetGetBuffer(long func)
{
    bufferCallBack = (getBuffer_call_back) func;
}

void BSDecoderSetFrameCallBack(long func)
{
    frameCallBack = (frame_call_back) func;
}

void BSDecoderSetInfoCallBackVideo(long func)
{
    videoInfoCallBack = (video_info_call_back) func;
}

void BSDecoderSetInfoCallBackAudio(long func)
{
    audioInfoCallBack = (audio_info_call_back) func;
}

void BSDecoderSetPollDataCallBack(long func)
{
    pollDataCallBack = (poll_data) func;
}

void BSDecoderSetErrorCallBack(long func)
{
    errorCallBack = (error_call_back) func;
}

void BSDecoderSetPollRawBufferCallback(long func)
{
    pollRawBuffer = (poll_raw_buffer) func;
}

#else

void BSDecoderSetReadCallBack(data_callback_read func)
{
    callbackRead = func;
}

void BSDecoderEOSCallBack(eos_call_back func)
{
    eosCallBack = func;
}

void BSDecoderSetGetBuffer(getBuffer_call_back func)
{
    bufferCallBack = func;
}

void BSDecoderSetFrameCallBack(frame_call_back func)
{
    frameCallBack = func;
}

void BSDecoderSetInfoCallBackVideo(video_info_call_back func)
{
    videoInfoCallBack = func;
}

void BSDecoderSetInfoCallBackAudio(audio_info_call_back func)
{
    audioInfoCallBack = func;
}


void BSDecoderSetPollDataCallBack(poll_data func)
{
    pollDataCallBack = func;
}
#endif

int BSDecoderPrepare()
{
    status = status_idle;
    readEos = false;
#ifdef __EMSCRIPTEN__
    decoder = std::unique_ptr<bitStreamDecoder>(new bitStreamDecoder(emscripten_read, listener));
#else
    decoder = std::unique_ptr<bitStreamDecoder>(new bitStreamDecoder(callbackRead, listener));
#endif
    return 0;
}

void BSDecoderStart()
{
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(pull_once, 100, 0);
#else
    pThread->start();
#endif
}

void BSDecoderFlush()
{
#ifndef __EMSCRIPTEN__
    pThread->pause();
#endif
    decoder->flush();

    if (status == status_eos) {
        status = status_prepared;
    }

    readEos = false;
}

void BSDecoderStop()
{
#ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop();
#else
    pThread->stop();
#endif
}

/*
 * ---------------------------------------------------
 */

static FILE *file = nullptr;
static bool g_eos{};

static int file_read(void *arg, uint8_t *buffer, int size)
{
    int ret = fread(buffer, 1, size, file);
    //  AF_LOGD("ret is %d\n", ret);
    return ret;
}

static void eosCB()
{
    g_eos = true;
}


static uint8_t *getBuffer(int size)
{
    return static_cast<uint8_t *>(malloc(size));
}

static void my_frame_call_back(uint8_t *buffer, int size, int pts, frameType type, int time)
{
    AF_LOGD("%s : pts %d\n", type == frameTypeVideo ? "Video" : "audio", pts);
    free(buffer);
}

static void my_video_info_call_back(int width, int height, int duration)
{
    AF_LOGD("video size is %d X %d\n", width, height);
}

void my_audio_info_call_back(int channels, int sample_rate, int duration)
{
    AF_LOGD("audio channels is %d ,sample_rate is %d\n", channels, sample_rate);
}

#ifndef __EMSCRIPTEN__
    int main()
#else

    int test()
#endif
{
    file = fopen("/Users/moqi/Downloads/sd1506863400_2.mp4", "r");

    if (file == nullptr) {
        AF_LOGE("cat open input file\n");
        return -1;
    }

#ifdef __EMSCRIPTEN__
    BSDecoderSetReadCallBack((long) file_read);
    BSDecoderEOSCallBack((long) eosCB);
    BSDecoderSetGetBuffer((long) getBuffer);
    BSDecoderSetFrameCallBack((long) my_frame_call_back);
    BSDecoderSetInfoCallBackVideo((long) my_video_info_call_back);
    BSDecoderSetInfoCallBackAudio((long) my_audio_info_call_back);
#else
    BSDecoderSetReadCallBack(file_read);
    BSDecoderEOSCallBack(eosCB);
    BSDecoderSetGetBuffer(getBuffer);
    BSDecoderSetFrameCallBack(my_frame_call_back);
    BSDecoderSetInfoCallBackVideo(my_video_info_call_back);
    BSDecoderSetInfoCallBackAudio(my_audio_info_call_back);
#endif
    g_eos = false;
    int ret = BSDecoderPrepare();

    if (ret < 0) {
        return ret;
    }

    BSDecoderStart();

    while (!g_eos) {
        af_usleep(10000);
    }

    BSDecoderStop();
    fclose(file);
#ifndef __EMSCRIPTEN__
    pThread = nullptr;
#endif
    decoder = nullptr;
    return 0;
}
