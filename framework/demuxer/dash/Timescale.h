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
            Timescale(uint64_t v = 0);

            int64_t ToTime(int64_t t) const;
            int64_t ToScaled(int64_t t) const;
            bool isValid() const;
            operator uint64_t() const;

        private:
            uint64_t scale;
        };
    }// namespace Dash
}// namespace Cicada


#endif//DEMUXER_DASH_TIMESCALE_H
