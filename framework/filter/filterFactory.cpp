//
// Created by moqi on 2019-08-23.
//

#include "filterFactory.h"
#include "ffmpegAudioFilter.h"

using namespace Cicada;

IAudioFilter *filterFactory::createAudioFilter(const IAudioFilter::format &srcFormat, const IAudioFilter::format &dstFormat,
                                               bool active = false)
{
    return new ffmpegAudioFilter(srcFormat, dstFormat, active);
}
