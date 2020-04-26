//
// Created by moqi on 2020/4/22.
//

#include "IProtocol.h"
#include <cstring>
#include <ifaddrs.h>
#include <netdb.h>
#include <sys/socket.h>
using namespace Cicada;

std::string IProtocolServer::getLocalIp()
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return "";
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;

        family = ifa->ifa_addr->sa_family;

        if (!strncmp(ifa->ifa_name, "lo", 2)) continue;
        if (family == AF_INET) {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
            if (s != 0) {
                return "";
            }
            freeifaddrs(ifaddr);
            return host;
        }
    }
    return "";
}
