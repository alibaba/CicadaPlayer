
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <pthread.h>

#ifdef  WIN32
#include <windows.h>
#else

#include <sys/time.h>
#include <unistd.h>

#endif

#ifdef __APPLE__

#include <TargetConditionals.h>
#include <string.h>

#endif

#ifdef ANDROID

#include <android/log.h>
#include <string.h>

#define ANDROID_APP_TAG "AliFrameWork"
#endif

#include "frame_work_log.h"

#undef printf

#ifndef VERSION
#define VERSION 0.9
#endif

#define str(a) #a
#define xstr(a) str(a)

#define NONE         "\033[m"
#define RED          "\033[0;32;31m"
#define LIGHT_RED    "\033[1;31m"
#define GREEN        "\033[0;32;32m"
#define LIGHT_GREEN  "\033[1;32m"
#define BLUE         "\033[0;32;34m"
#define LIGHT_BLUE   "\033[1;34m"
#define DARY_GRAY    "\033[1;30m"
#define CYAN         "\033[0;36m"
#define LIGHT_CYAN   "\033[1;36m"
#define PURPLE       "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN        "\033[0;33m"
#define YELLOW       "\033[1;33m"
#define LIGHT_GRAY   "\033[0;37m"
#define WHITE        "\033[1;37m"


typedef struct log_ctrl_t {
    int log_level;
    int disable_console;

    void (*log_out)(void *arg, int prio, const char *buf);

    void *log_out_arg;
    int disabled;

    void (*log_out2)(int prio, char *buf);

    void *log_out_arg2;
    int disabled2;
    void *userData;
    int enableColor;
} log_ctrl;

static log_ctrl logCtrl = {0,};

static pthread_mutex_t gLogMutex;

static const char *mtlVer = NULL;

static pthread_once_t once;
#ifndef ANDROID

pid_t gettid(void)
{
#ifdef __APPLE__
    return pthread_mach_thread_np(pthread_self());
#elif defined(WIN32)
    //TODO: get current thread id in windows platform
    return 0;
#else
    return pthread_self();
#endif
}

#endif

#ifdef ANDROID

static int get_android_lev(int prio)
{
    int lev;

    switch (prio) {
        case AF_LOG_LEVEL_DEBUG:

//            lev = ANDROID_LOG_DEBUG;
//            break;                //letv phone can't print debug info
        case AF_LOG_LEVEL_INFO:
            lev = ANDROID_LOG_INFO;
            break;

        case AF_LOG_LEVEL_WARNING:
            lev = ANDROID_LOG_WARN;
            break;

        case AF_LOG_LEVEL_ERROR:
            lev = ANDROID_LOG_ERROR;
            break;

        case AF_LOG_LEVEL_FATAL:
            lev = ANDROID_LOG_FATAL;
            break;

        default:
            lev = ANDROID_LOG_DEFAULT;
            break;
    }

    return lev;
}

#endif


static void get_local_time(char *buffer)
{
#ifdef WIN32
    SYSTEMTIME st = { 0 };
    GetLocalTime(&st);
    sprintf(buffer, "%d-%02d-%02d %02d:%02d:%02d %d",
            st.wYear,
            st.wMonth,
            st.wDay,
            st.wHour,
            st.wMinute,
            st.wSecond,
            st.wMilliseconds);
#else
    struct timeval t;
    gettimeofday(&t, NULL);
    struct tm *ptm = localtime(&t.tv_sec);
    sprintf(buffer, "%02d-%02d %02d:%02d:%02d.%03d",
            ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec,
            (int) (t.tv_usec / 1000));
    return;
#endif
}

static char get_char_lev(int lev, const char **ctrl)
{
    *ctrl = NONE;

    switch (lev) {
        case AF_LOG_LEVEL_INFO:
            *ctrl = GREEN;
            return 'I';

        case AF_LOG_LEVEL_DEBUG:
            *ctrl = NULL;
            return 'D';

        case AF_LOG_LEVEL_WARNING:
            *ctrl = YELLOW;
            return 'W';

        case AF_LOG_LEVEL_ERROR:
            *ctrl = RED;
            return 'E';

        case AF_LOG_LEVEL_FATAL:
            *ctrl = LIGHT_RED;
            return 'F';

        default:
            return ' ';
    }

    return ' ';
}

#ifndef ANDROID

static void linux_print(int prio, char *printf_buf)
{
    char time_buffer[32];
    char lev_c;
    const char *ctr = NULL;
    get_local_time(time_buffer);
    lev_c = get_char_lev(prio, &ctr);

    if (logCtrl.enableColor && ctr) {
        printf("%s", ctr);
    }

#ifdef WIN32
    OutputDebugString(printf_buf);
#endif
    printf("%s", printf_buf);

    if (ctr && logCtrl.enableColor) {
        printf(NONE);
    }
}

#endif

static void format_log(int prio, const char *tag, char *cont_buf, char *out_buf)
{
    char time_buffer[32];
    char lev_c;
#ifdef __APPLE__
    uint64_t atid;//pthread_mach_thread_np(pthread_self());
    pthread_threadid_np(NULL, &atid);
#else
    int tid = gettid();
#endif
    int pid = 0;
#ifndef WIN32
    pid = getpid();
#endif
    const char *ctr = NULL;
    get_local_time(time_buffer);
    lev_c = get_char_lev(prio, &ctr);
#ifdef __APPLE__
    sprintf(out_buf, "%s %d 0x%llx %c [%s] [%s]: %s", time_buffer, pid, atid, lev_c, mtlVer, tag,
            cont_buf);
#else
    sprintf(out_buf, "%s %d %d %c [%s] [%s]: %s", time_buffer, pid, tid, lev_c, mtlVer, tag,
            cont_buf);
#endif
    int len = (int) strlen(out_buf);

    if (out_buf[len - 1] != '\n') {
        out_buf[len] = '\n';
        out_buf[len + 1] = '\0';
    }

    return;
}

static void cicada_log_callback(int level, const char *buffer)
{
    if (buffer != NULL) {
        __log_print(level, "FFMPEG", "%s", buffer);
    }
}

static void initLog()
{
    if (mtlVer == NULL) {
        mtlVer = xstr(VERSION);
    }

#ifdef NDEBUG
    logCtrl.log_level = AF_LOG_LEVEL_INFO;
#else
    logCtrl.log_level = AF_LOG_LEVEL_DEBUG;
#endif
    //     regist_log_call_back(cicada_log_callback);
}

int __log_print(int prio, const char *tag, const char *fmt, ...)
{
    pthread_once(&once, initLog);
    if (prio > logCtrl.log_level) {
        return 0;
    }

    pthread_mutex_lock(&gLogMutex);
#ifdef ANDROID
    int lev = get_android_lev(prio);
#endif
    static char printf_buf[1024];
    static char out_buf[1024 + 256] = {0};
    va_list args;
    int printed;
    va_start(args, fmt);
    printed = vsnprintf(printf_buf, 1023, fmt, args);
    va_end(args);
    format_log(prio, tag, printf_buf, out_buf);

    if (logCtrl.log_out) {
        logCtrl.log_out(logCtrl.log_out_arg, prio, out_buf);
    }

    if (logCtrl.log_out2) {
        logCtrl.log_out2(prio, out_buf);
    }

    static char finalLogMsg[2048];
    sprintf(finalLogMsg, "[%s] [%s] :%s", mtlVer, tag, printf_buf);

    if (!logCtrl.disable_console) {
#ifdef ANDROID
        __android_log_print(lev, ANDROID_APP_TAG, "%s", finalLogMsg);
#else
        linux_print(prio, out_buf);
#endif
    }

    pthread_mutex_unlock(&gLogMutex);
    return 0;
}

void log_set_enable_console(int enable)
{
    logCtrl.disable_console = !enable;
}

void log_set_log_level(int level)
{
    pthread_once(&once, initLog);
    logCtrl.log_level = level;
    // cicada_set_log_level(level);
}

void log_set_level(int level, int enable_console)
{
    log_set_log_level(level);
    log_set_enable_console(enable_console);
}

int log_get_level()
{
    return logCtrl.log_level;
}

void log_set_back(log_back func, void *arg)
{
    logCtrl.log_out = func;
    logCtrl.log_out_arg = arg;
}

void log_enable_color(int enable)
{
    logCtrl.enableColor = enable;
}

char *getVersion()
{
    return (char *) mtlVer;
}

static char mtlTime[64];

char *getTime()
{
    snprintf(mtlTime, 64, "%s_%s", __DATE__, __TIME__);
    return mtlTime;
}
