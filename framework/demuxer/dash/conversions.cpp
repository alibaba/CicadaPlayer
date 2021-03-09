//
// Created by yuyuan on 2021/03/08.
//

#include "conversions.h"
#if !defined _WIN32
#include <locale>
#else
#include <time.h>
#include <windows.h>
#endif
#include <sstream>


double us_strtod(const char *str, char **end)
{
#if !defined _WIN32
    locale_t loc = newlocale(LC_NUMERIC_MASK, "C", nullptr);
    locale_t oldloc = uselocale(loc);
#endif
    double res = strtod(str, end);

#if !defined _WIN32
    if (loc != (locale_t) 0) {
        uselocale(oldloc);
        freelocale(loc);
    }
#endif
    return res;
}

static time_t str_duration(const char *psz_duration)
{
    bool timeDesignatorReached = false;
    time_t res = 0;
    char *end_ptr;

    if (psz_duration == nullptr) return -1;
    if ((*(psz_duration++)) != 'P') return -1;
    do {
        double number = us_strtod(psz_duration, &end_ptr);
        double mul = 0;
        if (psz_duration != end_ptr) psz_duration = end_ptr;
        switch (*psz_duration) {
            case 'M': {
                //M can mean month or minutes, if the 'T' flag has been reached.
                //We don't handle months though.
                if (timeDesignatorReached == true) mul = 60.0;
                break;
            }
            case 'Y':
            case 'W':
                break;//Don't handle this duration.
            case 'D':
                mul = 86400.0;
                break;
            case 'T':
                timeDesignatorReached = true;
                break;
            case 'H':
                mul = 3600.0;
                break;
            case 'S':
                mul = 1.0;
                break;
            default:
                break;
        }
        res += (time_t)(mul * number);
        if (*psz_duration) psz_duration++;
    } while (*psz_duration);
    return res;
}
int64_t Cicada::IsoTimeGetUs(const std::string &str)
{
    return str_duration(str.c_str()) * 1000000;
}

int64_t Cicada::UTCTimeGetUS(const std::string &str)
{
    enum { UTCTIME_YEAR = 0, UTCTIME_MON, UTCTIME_DAY, UTCTIME_HOUR, UTCTIME_MIN, UTCTIME_SEC, UTCTIME_MSEC, UTCTIME_TZ };
    int values[8] = {0};
    std::istringstream in(str);
    in.imbue(std::locale("C"));
    int64_t t;

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
            int i, tz = (in.peek() == '+') ? -60 : +60;
            in.ignore(1);
            if (!in.eof()) {
                in >> i;
                tz *= i;
                in.ignore(1);
                if (!in.eof()) {
                    in >> i;
                    tz += i;
                }
                values[UTCTIME_TZ] = tz;
            }
        }

        if (!in.fail() && !in.bad()) {
            struct tm tm;

            tm.tm_year = values[UTCTIME_YEAR] - 1900;
            tm.tm_mon = values[UTCTIME_MON] - 1;
            tm.tm_mday = values[UTCTIME_DAY];
            tm.tm_hour = values[UTCTIME_HOUR];
            tm.tm_min = values[UTCTIME_MIN];
            tm.tm_sec = values[UTCTIME_SEC];
            tm.tm_isdst = 0;

#if defined _WIN32
            int64_t mst = _mkgmtime(&tm);
#else
            int64_t mst = timegm(&tm);
#endif
            mst += values[UTCTIME_TZ] * 60;
            mst *= 1000;
            mst += values[UTCTIME_MSEC];
            t = 1000 * mst;
        } else {
            // Failure parsing time string
            t = 0;
        }
    } catch (...) {
        t = 0;
    }

    return t;
}
