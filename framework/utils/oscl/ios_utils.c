#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/host_info.h>
#include <mach/mach_host.h>
//#include <mach/task_info.h>
#include <mach/task.h>
#include "ios_utils.h"
#define LOG_TAG "ios_utils"
#include <utils/frame_work_log.h>

int get_system_meminfo(mem_info *pInfo)
{
    int mib[2];
    mib[0] = CTL_HW;
    mib[1] = HW_PAGESIZE;
    int pagesize;
    size_t length;
    length = sizeof(pagesize);

    if (sysctl(mib, 2, &pagesize, &length, NULL, 0) < 0) {
        AF_LOGE("getting page size");
        return -1;
    }

    // Total physical memory.
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    uint64_t physmem;
    size_t len = sizeof(physmem);

    if (sysctl(mib, 2, &physmem, &len, NULL, 0) == 0 && len == sizeof(physmem)) {
        pInfo->system_totalram = physmem;
    } else
        return -1;

    mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
    vm_statistics_data_t vmstat;

    if (host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t) &vmstat, &count) != KERN_SUCCESS) {
        AF_LOGE("Failed to get VM statistics.");
        return -1;
    }

    task_basic_info_64_data_t info;
    unsigned size = sizeof(info);
    task_info(mach_task_self (), TASK_BASIC_INFO_64, (task_info_t) &info, &size);
//    double total = pInfo->system_totalram;//(vmstat.wire_count + vmstat.active_count + vmstat.inactive_count + vmstat.free_count) * pagesize / unit;
//    double wired = vmstat.wire_count * pagesize / unit;
//    double active = vmstat.active_count * pagesize / unit;
    double inactive = vmstat.inactive_count * pagesize;
    double free = vmstat.free_count * pagesize;
    double resident = info.resident_size;
    pInfo->system_availableram = (uint64_t) (free + inactive);
    pInfo->system_freeram = (uint64_t) free;
    pInfo->self_useram = (uint64_t) resident;
    return 0;
}
