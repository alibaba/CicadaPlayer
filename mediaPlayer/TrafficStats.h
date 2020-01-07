//
// Created by lifujun on 2018/5/27.
//

#ifndef SOURCE_TRAFFICSTATS_H
#define SOURCE_TRAFFICSTATS_H


#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


static const uint64_t VALUE_UNKNOWN = -1;
static const char *IFACE_STAT_ALL = "/proc/net/xt_qtaguid/iface_stat_all";


enum Tx_Rx {
    TX,
    RX
};

enum Tcp_Udp {
    TCP,
    UDP,
    TCP_AND_UDP
};

// NOTE: keep these in sync with TrafficStats.java
enum IfaceStatType {
    RX_BYTES = 0,
    RX_PACKETS = 1,
    TX_BYTES = 2,
    TX_PACKETS = 3
};

struct IfaceStat {
    uint64_t rxBytes;
    uint64_t rxPackets;
    uint64_t txBytes;
    uint64_t txPackets;
};


uint64_t getIfaceStatType(const char *iface, enum IfaceStatType type);
int parseIfaceStat(const char *iface, struct IfaceStat *stat);

#ifdef __cplusplus
} //end extern "C" {
#endif

#endif //SOURCE_TRAFFICSTATS_H
