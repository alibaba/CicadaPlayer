//
// Created by moqi on 2019/11/14.
//

#include "gtest/gtest.h"
#include <memory>
#include <utils/frame_work_log.h>
#include <render/renderFactory.h>
#include <data_source/dataSourcePrototype.h>
#include <demuxer/demuxer_service.h>
#include <codec/decoderFactory.h>
#include <utils/timer.h>
#ifdef ENABLE_SDL
    #include <SDL2/SDL.h>
#endif
#include <utils/AFUtils.h>

using namespace std;
using namespace Cicada;

static void test_render(const string &url, Stream_type type, int flags, int samples)
{
    unique_ptr<IDecoder> decoder{nullptr};
    unique_ptr<IAudioRender> audioRender{nullptr};
    unique_ptr<IVideoRender> videoRender{nullptr};
#ifdef ENABLE_SDL
    SDL_Window *window = nullptr;
    SDL_Renderer *mVideoRender = nullptr;
#endif
    int samples_rendered = 0;
    auto source = dataSourcePrototype::create(url);
    source->Open(0);
    auto *demuxer = new demuxer_service(source);
    int ret = demuxer->initOpen();
    Stream_meta smeta{};
    unique_ptr<streamMeta> meta = unique_ptr<streamMeta>(new streamMeta(&smeta));

    for (int i = 0; i < demuxer->GetNbStreams(); ++i) {
        demuxer->GetStreamMeta(meta, i, false);

        if (((Stream_meta *) (*meta))->type == type) {
            demuxer->OpenStream(i);
            decoder = decoderFactory::create(((Stream_meta *) (*meta))->codec, flags, 0);
            ret = decoder->open(((Stream_meta *) (*meta)), nullptr, 0);
            break;
        }
    }

    std::unique_ptr<IAFPacket> packet{nullptr};
    unique_ptr<IAFFrame> frame{nullptr};
#ifdef ENABLE_SDL
    SDL_Event event;
#endif

    do {
        if (type == STREAM_TYPE_VIDEO) {
#ifdef ENABLE_SDL

            if (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    break;
                }
            }

#endif
        }

        if (frame == nullptr) {
            int decoder_ret;
            ret = demuxer->readPacket(packet, 0);

            if (ret > 0) {
                decoder_ret = decoder->send_packet(packet, 0);
            }

            if (ret == -EAGAIN) {
                af_msleep(10);
                ret = 0;
                continue;
            }

            decoder_ret = decoder->getFrame(frame, 0);

            if (decoder_ret == -EAGAIN) {
                af_msleep(10);
                continue;
            }
        } else {
            af_msleep(10);
        }

        if (frame) {
            if (type == STREAM_TYPE_AUDIO) {
                if (audioRender == nullptr) {
                    audioRender = AudioRenderFactory::create();
                    ASSERT_TRUE(audioRender);
                    ret = audioRender->init(&(frame->getInfo().audio));
                    ASSERT_GE(ret, 0);
                }

                audioRender->renderFrame(frame, 0);
            } else if (type == STREAM_TYPE_VIDEO) {
                if (videoRender == nullptr) {
                    videoRender = videoRenderFactory::create();
                    ASSERT_TRUE(videoRender);
#ifdef ENABLE_SDL
                    SDL_Init(SDL_INIT_VIDEO);
                    Uint32 flags = 0;
                    flags |= SDL_WINDOW_ALLOW_HIGHDPI;
                    flags |= SDL_WINDOW_RESIZABLE;
                    SDL_CreateWindowAndRenderer(1280, 720, flags, &window, &mVideoRender);
                    videoRender->setDisPlay(window);
#else
                    int x;
                    videoRender->setDisPlay(&x);
#endif
                    ret = videoRender->init();
                    ASSERT_GE(ret, 0);
                }

                af_usleep(frame->getInfo().duration);
                videoRender->renderFrame(frame);
            } else {
                frame = nullptr;
            }

            if (frame == nullptr) {
                if (++samples_rendered > samples) {
                    break;
                }
            }
        }
    } while (ret >= 0);

    delete source;
    delete demuxer;
#ifdef ENABLE_SDL

    if (mVideoRender != nullptr) {
        SDL_DestroyRenderer(mVideoRender);
        mVideoRender = nullptr;
    }

    if (window != nullptr) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

#endif
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ignore_signal(SIGPIPE);
    return RUN_ALL_TESTS();
}

TEST(audio, pcm_44100_2_s16)
{
    std::unique_ptr<IAudioRender> render = AudioRenderFactory::create();
    IAFFrame::audioInfo info{};
    info.sample_rate = 44100;
    info.channels = 2;
    info.format = AF_SAMPLE_FMT_S16;
    int ret = render->init(&info);
    ASSERT_GE(ret, 0);
}

TEST(audio, pcm_48000_2_s16)
{
    std::unique_ptr<IAudioRender> render = AudioRenderFactory::create();
    IAFFrame::audioInfo info{};
    info.sample_rate = 48000;
    info.channels = 2;
    info.format = AF_SAMPLE_FMT_S16;
    int ret = render->init(&info);
    ASSERT_GE(ret, 0);
}

TEST(audio, render)
{
    std::string url = "http://player.alicdn.com/video/aliyunmedia.mp4";
    test_render(url, STREAM_TYPE_AUDIO, DECFLAG_SW, 100);
}

TEST(video, render)
{
    std::string url = "http://player.alicdn.com/video/aliyunmedia.mp4";
    test_render(url, STREAM_TYPE_VIDEO, DECFLAG_SW, 100);
}
