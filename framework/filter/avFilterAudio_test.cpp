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
    ffmpegAudioFilter *filter{};
    unique_ptr<IAFFrame> frame{};
    bitStreamDecoder decoder{file_read, listener};
    SdlAFAudioRender audioRender{};
    decoder.prepare();

    while (listener.mFrames.empty()) {
        decoder.readPacket();
        decoder.pull_once();
    }

    filter = new ffmpegAudioFilter(listener.mFrames.front()->getInfo().audio, listener.mFrames.front()->getInfo().audio);
    filter->setOption("rate", "1", "atempo");
    filter->init();
    listener.mFrames.front()->dump();
    audioRender.init(&listener.mFrames.front()->getInfo().audio);
    filter->push(listener.mFrames.front(), 0);
    listener.mFrames.pop();

    while (!decoderEOS) {
        while (listener.mFrames.empty()) {
            decoder.readPacket();
            decoder.pull_once();
        }

        frame = nullptr;
        int ret = filter->pull(frame, 0);

        if (ret == -EAGAIN) {
            af_usleep(10000);
        }

        ret = filter->push(listener.mFrames.front(), 0);

        if (ret == -EAGAIN) {
            af_usleep(10000);
        }

        if (frame) {
//            uint8_t *buffer = dynamic_cast<AVAFFrame *>(frame.get())->ToAVFrame()->extended_data[0];
//            int size = dynamic_cast<AVAFFrame *>(frame.get())->ToAVFrame()->linesize[0];
            //  frame->dump();
            if (sample_count++ == 200) {
                filter->setOption("rate", "2", "atempo");
            }

            do {
                ret = audioRender.renderFrame(frame, 0);

                if (ret == -EAGAIN) {
                    af_msleep(10);
                }
            } while (ret == -EAGAIN);
        }

        if (listener.mFrames.front() == nullptr) {
            listener.mFrames.pop();
        } else {
            af_usleep(10000);
        }
    }

    return 0;
}

