//
// Created by pingkai on 2021/3/9.
//

#include "UTCTimer.h"
#include <cinttypes>
#include <iomanip>
#include <sstream>
using namespace Cicada;
using namespace std;
class UTCTime {
public:
    explicit UTCTime(const string &);
    uint64_t mtime() const;
    time_t time() const;

private:
    uint64_t t;
};

UTCTime::UTCTime(const string &str)
{
    enum { UTCTIME_YEAR = 0, UTCTIME_MON, UTCTIME_DAY, UTCTIME_HOUR, UTCTIME_MIN, UTCTIME_SEC, UTCTIME_MSEC, UTCTIME_TZ };
    int values[8] = {0};
    istringstream in(str);
    in.imbue(std::locale("C"));

    try {
        /* Date */
        for (int i = UTCTIME_YEAR; i <= UTCTIME_DAY && !in.eof(); i++) {
            if (i != UTCTIME_YEAR) in.ignore(1);
            in >> values[i];
        }
        /* Time */
        if (!in.eof() && in.peek() == 'T') {
            for (int i = UTCTIME_HOUR; i <= UTCTIME_SEC && !in.eof(); i++) {
                in.ignore(1);
                in >> values[i];
            }
        }
        if (!in.eof() && in.peek() == '.') {
            in.ignore(1);
            in >> values[UTCTIME_MSEC];
        }
        /* Timezone */
        if (!in.eof() && in.peek() == 'Z') {
            in.ignore(1);
        } else if (!in.eof() && (in.peek() == '+' || in.peek() == '-')) {
            int sign = (in.peek() == '+') ? 1 : -1;
            int tz = 0;
            in.ignore(1);

            if (!in.eof()) {
                string tzspec;
                in >> tzspec;

                if (tzspec.length() >= 4) {
                    tz = sign * std::stoul(tzspec.substr(0, 2)) * 60;
                    if (tzspec.length() == 5 && tzspec.find(':') == 2)
                        tz += sign * std::stoul(tzspec.substr(3, 2));
                    else
                        tz += sign * std::stoul(tzspec.substr(2, 2));
                } else {
                    tz = sign * std::stoul(tzspec) * 60;
                }
                values[UTCTIME_TZ] = tz;
            }
        }

        if (!in.fail() && !in.bad()) {
            struct tm tm {
            };

            tm.tm_year = values[UTCTIME_YEAR] - 1900;
            tm.tm_mon = values[UTCTIME_MON] - 1;
            tm.tm_mday = values[UTCTIME_DAY];
            tm.tm_hour = values[UTCTIME_HOUR];
            tm.tm_min = values[UTCTIME_MIN];
            tm.tm_sec = values[UTCTIME_SEC];
            tm.tm_isdst = 0;

            time_t mst = timegm(&tm);
            mst += values[UTCTIME_TZ] * -60;
            mst *= 1000;
            mst += values[UTCTIME_MSEC];
            t = mst;
        } else {
            // Failure parsing time string
            t = 0;
        }
    } catch (...) {
        t = 0;
    }
}

uint64_t UTCTime::mtime() const
{
    return t;
}
time_t UTCTime::time() const
{
    return t / 1000;
}
UTCTimer::UTCTimer(const std::string &time)
{
    UTCTime utcTime(time);
    mClock.set(utcTime.mtime() * 1000);
}
uint64_t UTCTimer::get()
{
    return mClock.get();
}
UTCTimer::operator std::string()
{
#define BUFLEN 255
    time_t t = mClock.get() / 1000000;
    char tmpBuf[BUFLEN];
    strftime(tmpBuf, BUFLEN, "%Y-%m-%d %H:%M:%S", gmtime(&t));
    return string(tmpBuf);
}
void UTCTimer::start()
{
    mClock.start();
}
