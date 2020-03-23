//
// Created by moqi on 2018/11/8.
//

#include <codec/avcodecDecoder.h>
#include "videoDecoderFactory.h"

namespace Cicada {
    static std::deque<IVideoDecoderFactory *> mFactoryQueue;
    static std::mutex gMutex;

    void VideoDecoderFactoryManager::registerFactory(IVideoDecoderFactory *factory)
    {
        std::lock_guard<std::mutex> lock(gMutex);

        for (int i = 0; i < mFactoryQueue.size(); ++i) {
            if (mFactoryQueue[i] == factory) {
                return;
            }
        }

        mFactoryQueue.push_back(factory);
    }

    IDecoder *VideoDecoderFactoryManager::create(enum AFCodecID codec, int flag, bool buildIn)
    {
        if (buildIn) {
#ifdef ENABLE_AVCODEC_DECODER

            if (avcodecDecoder::is_supported(codec)) {
                return new avcodecDecoder();
            }

#endif
            return nullptr;
        }

        std::lock_guard<std::mutex> lock(gMutex);

        for (int i = 0; i < mFactoryQueue.size(); ++i) {
            if (mFactoryQueue[i]->isSupport(codec)) {
                return mFactoryQueue[i]->create(codec, flag);
            }
        }

        return nullptr;
    }
}
