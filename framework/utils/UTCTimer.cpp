//
// Created by pingkai on 2021/3/9.
//
#define LOG_TAG "UTCTime"
#include "UTCTimer.h"
#include "frame_work_log.h"
#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <sys/types.h>
#include <utility>
#ifdef WIN32
#include <winsock.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif


#define NTP_TIMESTAMP_DELTA 2208988800ull
using namespace Cicada;
using namespace std;
class UTCTime {
public:
    explicit UTCTime(const string &);
    uint64_t mtime() const;
    time_t time() const;

private:
    uint64_t t;
};
typedef struct ntp_packet_t {

    uint8_t li_vn_mode;// Eight bits. li, vn, and mode.
    // li.   Two bits.   Leap indicator.
    // vn.   Three bits. Version number of the protocol.
    // mode. Three bits. Client will pick mode 3 for client.

    uint8_t stratum;  // Eight bits. Stratum level of the local clock.
    uint8_t poll;     // Eight bits. Maximum interval between successive messages.
    uint8_t precision;// Eight bits. Precision of the local clock.

    uint32_t rootDelay;     // 32 bits. Total round trip delay time.
    uint32_t rootDispersion;// 32 bits. Max error aloud from primary clock source.
    uint32_t refId;         // 32 bits. Reference clock identifier.

    uint32_t refTm_s;// 32 bits. Reference time-stamp seconds.
    uint32_t refTm_f;// 32 bits. Reference time-stamp fraction of a second.

    uint32_t origTm_s;// 32 bits. Originate time-stamp seconds.
    uint32_t origTm_f;// 32 bits. Originate time-stamp fraction of a second.

    uint32_t rxTm_s;// 32 bits. Received time-stamp seconds.
    uint32_t rxTm_f;// 32 bits. Received time-stamp fraction of a second.

    uint32_t txTm_s;// 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
    uint32_t txTm_f;// 32 bits. Transmit time-stamp fraction of a second.

} ntp_packet;

UTCTime::UTCTime(const string &str)
{
    enum { UTCTIME_YEAR = 0, UTCTIME_MON, UTCTIME_DAY, UTCTIME_HOUR, UTCTIME_MIN, UTCTIME_SEC, UTCTIME_MSEC, UTCTIME_TZ };
    int values[8] = {0};
    istringstream in(str);
    in.imbue(std::locale("C"));

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
            int sign = (in.peek() == '+') ? 1 : -1;
            int tz = 0;
            in.ignore(1);

            if (!in.eof()) {
                string tzspec;
                in >> tzspec;

                if (tzspec.length() >= 4) {
                    tz = sign * std::stoul(tzspec.substr(0, 2)) * 60;
                    if (tzspec.length() == 5 && tzspec.find(':') == 2)
                        tz += sign * std::stoul(tzspec.substr(3, 2));
                    else
                        tz += sign * std::stoul(tzspec.substr(2, 2));
                } else {
                    tz = sign * std::stoul(tzspec) * 60;
                }
                values[UTCTIME_TZ] = tz;
            }
        }

        if (!in.fail() && !in.bad()) {
            struct tm tm {
            };

            tm.tm_year = values[UTCTIME_YEAR] - 1900;
            tm.tm_mon = values[UTCTIME_MON] - 1;
            tm.tm_mday = values[UTCTIME_DAY];
            tm.tm_hour = values[UTCTIME_HOUR];
            tm.tm_min = values[UTCTIME_MIN];
            tm.tm_sec = values[UTCTIME_SEC];
            tm.tm_isdst = 0;

#if defined _WIN32
            time_t mst = _mkgmtime(&tm);
#else
            time_t mst = timegm(&tm);
#endif
            mst += values[UTCTIME_TZ] * -60;
            mst *= 1000;
            mst += values[UTCTIME_MSEC];
            t = mst;
        } else {
            // Failure parsing time string
            t = 0;
        }
    } catch (...) {
        t = 0;
    }
}

uint64_t UTCTime::mtime() const
{
    return t;
}
time_t UTCTime::time() const
{
    return t / 1000;
}
UTCTimer::UTCTimer(const std::string &time)
{
    UTCTime utcTime(time);
    mClock.set(utcTime.mtime() * 1000);
}
int64_t UTCTimer::get()
{
    return mClock.get();
}
UTCTimer::operator std::string()
{
#define BUFLEN 255
    time_t t = mClock.get() / 1000000;
    char tmpBuf[BUFLEN];
    strftime(tmpBuf, BUFLEN, "%Y-%m-%d %H:%M:%S", gmtime(&t));
    return string(tmpBuf);
}
void UTCTimer::start()
{
    mClock.start();
}
NTPClient::NTPClient(string server, int64_t port) : mServer(std::move(server)), mPort(port)
{
    if (!mThread) {
        mThread = unique_ptr<afThread>(NEW_AF_THREAD(getNTPTime));
        mThread->start();
    }
}
NTPClient::~NTPClient() = default;
int64_t NTPClient::get() const
{
    if (mTime == 0) {
        return -EAGAIN;
    } else if (mTime < 0) {
        return mTime;
    }
    return mTime;
}

int64_t NTPClient::getTimeSync() const
{
    if (mThread) {
        mThread->stop();
    }
    return mTime;
}

NTPClient::NTPClient()
{
    if (!mThread) {
        mThread = unique_ptr<afThread>(NEW_AF_THREAD(getNTPTime));
        mThread->start();
    }
};
int NTPClient::getNTPTime()
{
    ntp_packet packet = {
            0,
    };
    int sockfd, n;
    *((char *) &packet + 0) = 0x1b;

    struct sockaddr_in serv_addr {
    };
    struct hostent *server;
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sockfd < 0) {
        AF_LOGE("ERROR opening socket");
        mTime = -errno;
        return -1;
    }

    server = gethostbyname(mServer.c_str());

    if (server == nullptr) {
        AF_LOGE("ERROR, no such host");
        mTime = -errno;
        return -1;
    }
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *) &serv_addr.sin_addr.s_addr, (char *) server->h_addr, server->h_length);
    serv_addr.sin_port = htons(mPort);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        AF_LOGE("ERROR connecting");
        mTime = -errno;
        return -1;
    }
    n = send(sockfd, (char *) &packet, sizeof(ntp_packet), 0);
    if (n < 0) {
        AF_LOGE("ERROR writing to socket");
        mTime = -errno;
        return -1;
    }
    n = recv(sockfd, (char *) &packet, sizeof(ntp_packet), 0);
    if (n < 0) {
        AF_LOGE("ERROR reading from socket");
        mTime = -errno;
        return -1;
    }
    packet.txTm_s = ntohl(packet.txTm_s);
    packet.txTm_f = ntohl(packet.txTm_f);

    mTime = (packet.txTm_s - NTP_TIMESTAMP_DELTA);

    AF_LOGD("Time: %s", ctime((const time_t *) &mTime));
    return -1;
}
NTPClient::operator std::string()
{
#define BUFLEN 255
    if (mTime <= 0) {
        return "";
    }
    char tmpBuf[BUFLEN];
    strftime(tmpBuf, BUFLEN, "%Y-%m-%dT%H:%M:%SZ", gmtime(&mTime));
    return string(tmpBuf);
}
