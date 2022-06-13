//
// Created by pingkai on 2021/3/9.
//

#ifndef CICADAMEDIA_UTCTIMER_H
#define CICADAMEDIA_UTCTIMER_H

#include "afThread.h"
#include "af_clock.h"
#include <string>
namespace Cicada {

    class UTCTime {
    public:
        explicit UTCTime(const std::string &);
        explicit UTCTime(uint64_t);
        uint64_t mtime() const;
        uint64_t time() const;

    private:
        uint64_t t;//us
    };

    class UTCTimer {
    public:
        explicit UTCTimer(const std::string &time);
        explicit UTCTimer(uint64_t timeMs);
        ~UTCTimer() = default;
        int64_t get();
        void start();
        void setTime(const std::string &time);
        void setTime(uint64_t timeMs);
        explicit operator std::string();

    private:
        af_clock mClock;
    };

    class NTPClient {
    public:
        NTPClient(std::string server, int64_t port = 123);
        NTPClient();
        ~NTPClient();
        int64_t get() const;

        int64_t getTimeSync(int timeoutMs) const;
        explicit operator std::string();

    private:
        int getNTPTime();

    private:
        std::string mServer = "ntp.aliyun.com";
        int64_t mPort = 123;
        std::unique_ptr<afThread> mThread{};
        int64_t mTime{0};
    };
}// namespace Cicada


#endif//CICADAMEDIA_UTCTIMER_H
