//
// Created by moqi on 2019-07-29.
//

#include <WebAssemblyPlayer/bitStreamDecoder.h>
#include <utils/ffmpeg_utils.h>
#include <filter/ffmpegAudioFilter.h>
#include <utils/timer.h>
#include <render/audio/IAudioRender.h>
#include <render/audio/SdlAFAudioRender.h>

using namespace Cicada;

bool readEos = false;
bool decoderEOS = false;

class filterListener : public bitStreamDecoder::eventListener
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

        decoderEOS = true;
    };

    void onFrame(IAFFrame *frame, int consumeTime) override
    {
        IAFFrame::FrameType type = frame->getType();
        int size = 0;

        if (type == IAFFrame::FrameTypeVideo) {
        } else {
            mFrames.push(frame->clone());
        }
    };

    void onStreamMeta(const Stream_meta *meta) override
    {
//        if (meta->type == STREAM_TYPE_VIDEO) {
//
//        } else if (meta->type == STREAM_TYPE_AUDIO) {
//
//        }
    }

public:


    std::queue<std::unique_ptr<IAFFrame>> mFrames{};

};

static filterListener listener{};
static FILE *file = nullptr;

static int file_read(void *arg, uint8_t *buffer, int size)
{
    int ret = fread(buffer, 1, size, file);
    return ret;
}


int main()
{
    int sample_count = 0;
    file = fopen("/Users/moqi/Downloads/aliyunmedia.mp4", "r");
    unique_ptr<IAFFrame> frame{};
    bitStreamDecoder decoder{file_read, listener};
    SdlAFAudioRender audioRender{};
    decoder.prepare();

    while (listener.mFrames.empty()) {
        decoder.readPacket();
        decoder.pull_once();
    }

    listener.mFrames.front()->dump();
    audioRender.init(&listener.mFrames.front()->getInfo().audio);
    audioRender.setSpeed(1.5);

    while (!decoderEOS) {
        while (listener.mFrames.empty()) {
            decoder.readPacket();
            decoder.pull_once();
        }

        int ret = audioRender.renderFrame(listener.mFrames.front(), 0);

        if (ret == -EAGAIN) {
            af_msleep(10);
            continue;
        }

        listener.mFrames.pop();

        if (sample_count++ == 200) {
            audioRender.setSpeed(2);
        }
    }

    return 0;
}

