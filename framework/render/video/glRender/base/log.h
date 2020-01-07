#ifndef LOG_H
#define LOG_H

//#define LOG_TAG "render_engine"

#include <utils/frame_work_log.h>
namespace cicada
{
#define RLOGD(fmt, ...)    __log_print(AF_LOG_LEVEL_DEBUG,LOG_TAG,fmt, ##__VA_ARGS__)
#define RLOGI(fmt, ...)    __log_print(AF_LOG_LEVEL_INFO,LOG_TAG,fmt, ##__VA_ARGS__)
#define RLOGW(fmt, ...)   __log_print(AF_LOG_LEVEL_WARNING,LOG_TAG,fmt, ##__VA_ARGS__)
#define RLOGE(fmt, ...)    __log_print(AF_LOG_LEVEL_ERROR,LOG_TAG,fmt, ##__VA_ARGS__)


} // namespace cicada
#endif // LOG_H
