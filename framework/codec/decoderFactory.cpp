//
// Created by moqi on 2019-08-20.
//

#include "decoderFactory.h"

#ifdef ANDROID

    #include "Android/mediaCodecDecoder.h"

#endif

#ifdef __APPLE__

    #include "Apple/AppleVideoToolBox.h"

#endif

#include "avcodecDecoder.h"

using namespace Cicada;
using namespace std;

unique_ptr<Cicada::IDecoder> decoderFactory::create(AFCodecID codec, uint64_t flags, int maxSize)
{
    IDecoder *decoder = codecPrototype::create(codec, flags, maxSize);

    if (decoder != nullptr) {
        return unique_ptr<IDecoder>(decoder);
    }

    return createBuildIn(codec, flags);
}

unique_ptr<IDecoder> decoderFactory::createBuildIn(const AFCodecID &codec, uint64_t flags)
{
    if (flags & DECFLAG_HW) {
#ifdef ANDROID
        return std::unique_ptr<IDecoder>(new mediaCodecDecoder());
#endif
#ifdef __APPLE__

        if (AFVTBDecoder::is_supported(codec)) {
            return unique_ptr<IDecoder>(new AFVTBDecoder());
        }

#endif
    }

    if (flags & DECFLAG_SW) {
        return unique_ptr<IDecoder>(new avcodecDecoder());
    }

    return nullptr;
}

