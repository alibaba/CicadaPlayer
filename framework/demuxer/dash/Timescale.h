//
// Created by yuyuan on 2021/03/09.
//

#ifndef DEMUXER_DASH_TIMESCALE_H
#define DEMUXER_DASH_TIMESCALE_H

#include <stdint.h>

namespace Cicada {
    namespace Dash {
        class Timescale {
        public:
            Timescale(uint64_t v = 0) : scale(v)
            {}

            int64_t ToTime(int64_t t) const
            {
                if (!scale) return 0;
                int64_t v = t / scale;
                int64_t r = t % scale;
                return v * 1000000 + r * 1000000 / scale;
            }

            int64_t ToScaled(int64_t t) const
            {
                int64_t v = t / 1000000;
                int64_t r = t % 1000000;
                return v * scale + r * scale / 1000000;
            }

            bool isValid() const
            {
                return !!scale;
            }
            operator uint64_t() const
            {
                return scale;
            }

        private:
            uint64_t scale;
        };
    }// namespace Dash
}// namespace Cicada


#endif//DEMUXER_DASH_TIMESCALE_H
