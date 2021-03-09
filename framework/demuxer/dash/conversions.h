//
// Created by yuyuan on 2021/03/08.
//

#ifndef DEMUXER_DASH_CONVERSIONS_H
#define DEMUXER_DASH_CONVERSIONS_H

#include <cstdint>
#include <string>
namespace Cicada {
    int64_t IsoTimeGetUs(const std::string &);
    int64_t UTCTimeGetUS(const std::string &str);
}// namespace Cicada

#endif//DEMUXER_DASH_CONVERSIONS_H
