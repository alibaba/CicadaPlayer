#include <sys/sysinfo.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "linux_utils.h"
#define LOG_TAG "linux_utils"
//#include "utils/frame_work_log.h"
typedef struct {
    size_t	size;		/* total program size */
    size_t	resident;	/* resident set size */
    size_t	share;		/* shared pages (i.e., backed by a file) */
    size_t	text;		/* text (code) */
    size_t	lib;		/* library (unused in Linux 2.6 */
    size_t	data;		/* data + stack */
    size_t	dt;		/* dirty pages (unused in Linux 2.6 */
    size_t	pagesize;	/* pagesize */
} statm;

static FILE *procMeminfoFP = NULL;
// Can't use rewind
static int  procstatm (statm *usage)
{
    FILE *fd;
    usage->pagesize = 4096;//getpagesize();

    if ((fd = fopen("/proc/self/statm", "r")) == NULL) {
//        AF_LOGW("Can't open statm file");
        return -1;
    }

    if (fscanf(fd, "%zu %zu %zu %zu %zu %zu %zu",
               &(usage->size),
               &(usage->resident),
               &(usage->share),
               &(usage->text),
               &(usage->lib),
               &(usage->data),
               &(usage->dt)) != 7) {
//		AF_LOGE("scanf error\n");
        fclose(fd);
        return -1;
    };

    fclose(fd);

    return 0;
}
#ifdef ANDROID
static int32_t android_lowmemkillersize[6] = {0,};
#define FOREGROUD_APP    0
#define VISIBLE_APP      1
#define SECONDARY_SERVER 2
#define HOME_APP         4
#define HIDDEN_APP       7
#define CONTENT_PROVIDER 14
#define EMPTY_APP        15
int64_t android_get_low_mem(int level)
{
    char *path = "/sys/module/lowmemorykiller/parameters/minfree";
    FILE *fd;
    uint64_t size[6] = {0,};
    int lev[6] = {FOREGROUD_APP, VISIBLE_APP, SECONDARY_SERVER,
                  HIDDEN_APP, CONTENT_PROVIDER, EMPTY_APP
                 };
    int i = 0;

    if (android_lowmemkillersize[0] == 0) {
        if ((fd = fopen(path, "r")) == NULL) {
//            AF_LOGW("Can't open  file %s\n",path);
            return -1;
        }

        if (fscanf(fd, "%" PRId32 ",%" PRId32 ",%" PRId32 ",%" PRId32 ",%" PRId32 ",%" PRId32 "",
                   &(android_lowmemkillersize[0]),
                   &(android_lowmemkillersize[1]),
                   &(android_lowmemkillersize[2]),
                   &(android_lowmemkillersize[3]),
                   &(android_lowmemkillersize[4]),
                   &(android_lowmemkillersize[5])) != 6) {
//            AF_LOGE("scanf error\n");
            fclose(fd);
            return -1;
        };

        fclose(fd);
    }

    for (i = 0; i < 6; i++)
        if (level == lev[i])
            return (int64_t)android_lowmemkillersize[i] * 4096;

    return -1;
}
#endif
int get_system_meminfo(mem_info *pInfo)
{
    struct sysinfo info;
    char name[32];
    unsigned val;
    int ret;

    if (!procMeminfoFP && (procMeminfoFP = fopen("/proc/meminfo", "r")) == NULL)
        sysinfo(&info);
    else {
        memset(&info, 0, sizeof(struct sysinfo));
        info.mem_unit = 4096;

        //    unsigned long sharedram = 0;
        //    unsigned long
        while (fscanf(procMeminfoFP, "%31s %u%*[^\n]\n", name, &val) != EOF) {
            if (strncmp("MemTotal:", name, 9) == 0)
                info.totalram = val / 4;
            else if (strncmp("MemFree:", name, 8) == 0)
                info.freeram = val / 4;
            else if (strncmp("Buffers:", name, 8) == 0)
                info.bufferram += val / 4;
            else if (strncmp("Cached:", name, 7) == 0)
                info.bufferram += val / 4;
            else if (strncmp("SwapTotal:", name, 10) == 0)
                info.totalswap = val / 4;
            else if (strncmp("SwapFree:", name, 9) == 0)
                info.freeswap = val / 4;
            else if (strncmp("HighTotal:", name, 10) == 0)
                info.totalhigh = val / 4;
            else if (strncmp("HighFree:", name, 9) == 0)
                info.freehigh = val / 4;
            else if (strncmp("SwapCached:", name, 11) == 0) {
                info.bufferram -= val / 4;
            } else if (strncmp("Shmem:", name, 6) == 0) {
                info.bufferram -= val / 4;
            }
        }

        rewind(procMeminfoFP);
        fflush(procMeminfoFP);
    }

    pInfo->system_totalram    = (uint64_t)(info.totalram * info.mem_unit);
    pInfo->system_availableram = (uint64_t)((info.freeram + info.bufferram) * info.mem_unit);
    pInfo->system_freeram      = (uint64_t)(info.freeram * info.mem_unit);
//  pInfo->dwLength        = sizeof(MEMORYSTATUSEX);
//  pInfo->ullAvailPageFile = (info.freeswap * info.mem_unit);
//  pInfo->ullAvailPhys     = ((info.freeram + info.bufferram) * info.mem_unit);
//  pInfo->ullAvailVirtual  = ((info.freeram + info.bufferram) * info.mem_unit);
//  pInfo->ullTotalPhys     = (info.totalram * info.mem_unit);
//  pInfo->ullTotalVirtual  = (info.totalram * info.mem_unit);
    statm usage = {0,};
    ret = procstatm(&usage);
//    if (ret < 0)
//        return -1;
    pInfo->self_useram = (uint64_t)usage.resident * usage.pagesize;
    return 0;
}