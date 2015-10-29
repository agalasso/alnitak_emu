/*
 * Alnitak Flat-man emulator for Spike-a USB dimmer
 *
 * Copyright (c) 2015 Andy Galasso <andy.galasso@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of Andy Galasso, adgsoftware.com nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "dbg.h"
#include <stdarg.h>
#include <time.h>

namespace dbg {
    bool verbose;
}

#ifdef _MSC_VER

#include <windows.h>

struct timezone2
{
    __int32  tz_minuteswest; /* minutes W of Greenwich */
    bool  tz_dsttime;     /* type of dst correction */
};

static int
gettimeofday(struct timeval *tv, struct timezone2 *tz)
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    __int64 tmpres = ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;

    /*converting file time to unix epoch*/
    const __int64 DELTA_EPOCH_IN_MICROSECS = 11644473600000000;
    tmpres /= 10;  /*convert into microseconds*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS;
    tv->tv_sec = (__int32)(tmpres / 1000000);
    tv->tv_usec = (tmpres % 1000000);

    if (tz)
    {
        TIME_ZONE_INFORMATION tz_winapi;
        ZeroMemory(&tz_winapi, sizeof(tz_winapi));
        int rez = GetTimeZoneInformation(&tz_winapi);
        tz->tz_dsttime = (rez == 2) ? true : false;
        tz->tz_minuteswest = tz_winapi.Bias + ((rez == 2) ? tz_winapi.DaylightBias : 0);
    }

    return 0;
}

#else // _MSC_VER
# include <sys/time.h>
#endif // _MSC_VER

time64_t time64()
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    return (time64_t) tv.tv_sec * 1000ULL + tv.tv_usec / 1000;
}

#ifdef DBG_ENABLED

#ifdef _MSC_VER
# define snprintf _snprintf_s
# define sscanf sscanf_s
inline static void localtime_r(const time_t *t, struct tm *tm)
{
    localtime_s(tm, t);
}
#endif

static const char *
timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, 0);

    time_t t = tv.tv_sec;
    struct tm tm;
    localtime_r(&t, &tm);

    static char buf[128];
    snprintf(buf, sizeof(buf), "[%04u-%02u-%02u %02u:%02u:%02u.%03u] ",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec,
        tv.tv_usec / 1000);

    return buf;
}

void dbgpr(const char *format, ...)
{
    fputs(timestamp(), stderr);
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
}

#endif DBG_ENABLED
