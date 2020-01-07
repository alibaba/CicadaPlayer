//
// Created by lifujun on 2018/5/27.
//

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include "TrafficStats.h"


uint64_t getIfaceStatType(const char *iface, enum IfaceStatType type)
{
    struct IfaceStat stat;
    memset(&stat, 0, sizeof(struct IfaceStat));

    if (parseIfaceStat(iface, &stat)) {
        return VALUE_UNKNOWN;
    }

    switch (type) {
        case RX_BYTES:
            return stat.rxBytes;

        case RX_PACKETS:
            return stat.rxPackets;

        case TX_BYTES:
            return stat.txBytes;

        case TX_PACKETS:
            return stat.txPackets;

        default:
            return VALUE_UNKNOWN;
    }
}


int parseIfaceStat(const char *iface, struct IfaceStat *stat)
{
    FILE *fp = fopen(IFACE_STAT_ALL, "r");

    if (!fp) {
        return errno;
    }

    char     buffer[256];
    char     cur_iface[32];
    int      active;
    uint64_t rxBytes, rxPackets, txBytes, txPackets, devRxBytes, devRxPackets, devTxBytes,
             devTxPackets;

    while (fgets(buffer, 256, fp) != NULL) {
        if (sscanf(buffer,
                   "%31s %" PRId32 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "",
                   cur_iface, &active,
                   &rxBytes, &rxPackets, &txBytes, &txPackets, &devRxBytes, &devRxPackets,
                   &devTxBytes, &devTxPackets) != 10) {
            continue;
        }

        if (!iface || !strcmp(iface, cur_iface)) {
            stat->rxBytes += rxBytes;
            stat->rxPackets += rxPackets;
            stat->txBytes += txBytes;
            stat->txPackets += txPackets;

            if (active) {
                stat->rxBytes += devRxBytes;
                stat->rxPackets += devRxPackets;
                stat->txBytes += devTxBytes;
                stat->txPackets += devTxPackets;
            }
        }
    }

    fclose(fp);
    return 0;
}