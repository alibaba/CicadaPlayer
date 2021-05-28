//
// Created by yuyuan on 2021/03/09.
//

#include "Timescale.h"

using namespace Cicada::Dash;

Timescale::Timescale(uint64_t v) : scale(v)
{}

int64_t Timescale::ToTime(int64_t t) const
{
    if (!scale) return 0;
    int64_t v = t / scale;
    if (v > INT64_MAX / 1000000) {
        return 0;
    }
    int64_t r = t % scale;
    return v * 1000000 + r * 1000000 / scale;
}

int64_t Timescale::ToScaled(int64_t t) const
{
    int64_t v = t / 1000000;
    if (scale == 0 || v > INT64_MAX / scale) {
        return 0;
    }
    int64_t r = t % 1000000;
    return v * scale + r * scale / 1000000;
}

bool Timescale::isValid() const
{
    return !!scale;
}

Timescale::operator uint64_t() const
{
    return scale;
}