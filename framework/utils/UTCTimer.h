//
// Created by pingkai on 2021/3/9.
//

#ifndef CICADAMEDIA_UTCTIMER_H
#define CICADAMEDIA_UTCTIMER_H

#include "af_clock.h"
#include <string>
namespace Cicada {
    class UTCTimer {
    public:
        explicit UTCTimer(const std::string &time);
        ~UTCTimer() = default;
        uint64_t get();
        void start();
        explicit operator std::string();

    private:
        af_clock mClock;
    };
}// namespace Cicada


#endif//CICADAMEDIA_UTCTIMER_H
