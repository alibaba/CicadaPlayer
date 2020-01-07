// ffmpegshim.c
// this file is part of Context Free
// ---------------------
// Copyright (C) 2012 John Horigan - john@glyphic.com
// Copyright (C) 2000 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// John Horigan can be contacted at john@glyphic.com or at
// John Horigan, 1209 Villa St., Mountain View, CA 94041-1123, USA
//
//

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <math.h>

int __cdecl __mingw_vsnprintf(
        char *buffer,
        size_t count,
        const char *format,
        va_list argptr
) {
    return vsnprintf_s(buffer, count, _TRUNCATE, format, argptr);
}
int __cdecl __ms_vsnprintf(
        char *buffer,
        size_t count,
        const char *format,
        va_list argptr
) {
    return vsnprintf_s(buffer, count, _TRUNCATE, format, argptr);
}
int __cdecl __mingw_vfprintf(
        FILE *stream,
        const char *format,
        va_list argptr
) {
    return vfprintf_s(stream, format, argptr);
}

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

struct timezone {
    int tz_minuteswest; /* minutes W of Greenwich */
    int tz_dsttime;     /* type of dst correction */
};
struct timeval {
    long tv_sec;
    long tv_usec;
};

int __cdecl gettimeofday(
        struct timeval *tv,
        struct timezone *tz
) {
    FILETIME ft;
    unsigned __int64 tmpres = 0;
    static int tzflag;

    if (NULL != tv) {
        GetSystemTimeAsFileTime(&ft);

        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;

        /*converting file time to unix epoch*/
        tmpres -= DELTA_EPOCH_IN_MICROSECS;
        tmpres /= 10;  /*convert into microseconds*/
        tv->tv_sec = (long) (tmpres / 1000000UL);
        tv->tv_usec = (long) (tmpres % 1000000UL);
    }

    if (NULL != tz) {
        int i;
        long l = 0;
        if (!tzflag) {
            _tzset();
            tzflag++;
        }
        _get_timezone(&l);
        tz->tz_minuteswest = l / 60;
        _get_daylight(&i);
        tz->tz_dsttime = i;
    }

    return 0;
}

int __cdecl usleep(
        unsigned int useconds
) {
    if (useconds == 0)
        return 0;

    if (useconds >= 1000000)
        return EINVAL;

    Sleep((useconds + 999) / 1000);

    return 0;
}

double __cdecl __strtod(
        const char *nptr,
        char **endptr
) {
    return strtod(nptr, endptr);
}double __cdecl __mingw_strtod(
        const char *nptr,
        char **endptr
) {
    return strtod(nptr, endptr);
}

#if defined (_MSC_VER) && _MSC_VER < 1800
__int64 strtoll(
     const char *nptr,
     char **endptr,
     int base)
{
    return _strtoi64(nptr, endptr, base);
}

double __cdecl trunc(
     double n
)
{
    return n > 0.0 ? floor(n) : ceil(n);
}
#endif

int __cdecl __mingw_vsscanf(
        const char *s,
        const char *fmt,
        va_list ap
) {
    void *a[5];
    int i;
    for (i = 0; i < sizeof(a) / sizeof(a[0]); i++)
        a[i] = va_arg(ap, void *);
    return sscanf(s, fmt, a[0], a[1], a[2], a[3], a[4]);
}

unsigned __int64 __cdecl udivmodsi4(
        unsigned __int64 num,
        unsigned __int64 den,
        int modwanted
) {
    unsigned __int64 bit = 1;
    unsigned __int64 res = 0;

    while (den < num && bit && !(den & (1L << 31))) {
        den <<= 1;
        bit <<= 1;
    }
    while (bit) {
        if (num >= den) {
            num -= den;
            res |= bit;
        }
        bit >>= 1;
        den >>= 1;
    }
    return modwanted ? num : res;
}

//__int64 __cdecl __divdi3(
//        __int64 a,
//        __int64 b
//) {
//    int neg = 0;
//    __int64 res;
//
//    if (a < 0) {
//        a = -a;
//        neg = !neg;
//    }
//
//    if (b < 0) {
//        b = -b;
//        neg = !neg;
//    }
//
//    res = udivmodsi4(a, b, 0);
//
//    if (neg)
//        res = -res;
//
//    return res;
//}
//
//__int64 __cdecl __moddi3(
//        __int64 a,
//        __int64 b
//) {
//    int neg = 0;
//    __int64 res;
//
//    if (a < 0) {
//        a = -a;
//        neg = 1;
//    }
//
//    if (b < 0)
//        b = -b;
//
//    res = udivmodsi4(a, b, 1);
//
//    if (neg)
//        res = -res;
//
//    return res;
//}
//
//unsigned __int64 __cdecl __udivdi3(
//        unsigned __int64 a,
//        unsigned __int64 b
//) {
//    return udivmodsi4(a, b, 0);
//}
//
//unsigned __int64 __cdecl __umoddi3(
//        unsigned __int64 a,
//        unsigned __int64 b
//) {
//    return udivmodsi4(a, b, 1);
//}
typedef int (__cdecl *fUserMathErr)(struct _exception *);
static fUserMathErr stUserMathErr;
void __mingw_raise_matherr (int typ, const char *name, double a1, double a2,
                            double rslt)
{
    struct _exception ex;
    if (!stUserMathErr)
        return;
    ex.type = typ;
    ex.name = (char*)name;
    ex.arg1 = a1;
    ex.arg2 = a2;
    ex.retval = rslt;
    (*stUserMathErr)(&ex);
}

// TODO:
static int nerr = 0;
int *_imp___sys_nerr(void){
    return &nerr;
}
