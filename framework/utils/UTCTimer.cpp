//
// Created by pingkai on 2021/3/9.
//
#define LOG_TAG "UTCTime"
#include "UTCTimer.h"
#include "frame_work_log.h"
#include "utils/timer.h"
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
    explicit UTCTime(time_t);
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

UTCTime::UTCTime(time_t t_)
{
    t = t_ * 1000;
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
    int64_t t = af_getsteady_ms();
    UTCTime utcTime(time);
    int64_t delta = af_getsteady_ms() - t;
    mClock.set((utcTime.mtime() + delta) * 1000);
}
UTCTimer::UTCTimer(time_t time)
{
    int64_t t = af_getsteady_ms();
    UTCTime utcTime(time);
    int64_t delta = af_getsteady_ms() - t;
    mClock.set((utcTime.mtime() + delta) * 1000);
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
    size_t len = strftime(tmpBuf, BUFLEN, "%Y-%m-%d %H:%M:%S", gmtime(&t));
    sprintf(tmpBuf + len, ".%03d", (int) (mClock.get() % 1000000) / 1000);
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
NTPClient::~NTPClient()
{
    mThread->detach();
}
int64_t NTPClient::get() const
{
    if (mTime == 0) {
        return -EAGAIN;
    }
    return mTime;
}

int64_t NTPClient::getTimeSync(int timeoutMs) const
{
    int64_t time;
    int tryTimes = timeoutMs / 100;
    do {
        time = get();
        if (time == -EAGAIN) {
            af_msleep(100);
        }
        tryTimes--;
    } while (time == -EAGAIN && tryTimes >= 0);
    return time;
}

NTPClient::NTPClient()
{
    if (!mThread) {
        mThread = unique_ptr<afThread>(NEW_AF_THREAD(getNTPTime));
        mThread->start();
    }
};

#define JAN_1970 0x83aa7e80
#define NTPFRAC(x) (4294 * (x) + ((1981 * (x)) >> 11))
#define USEC(x) (((x) >> 12) - 759 * ((((x) >> 10) + 32768) >> 16))
#define Data(i) ntohl(((unsigned int *) data)[i])
#define LI 0
#define VN 3
#define MODE 3
#define STRATUM 0
#define POLL 4
#define PREC (-6)

typedef struct NtpTime_t {
    unsigned int coarse;
    unsigned int fine;
} NtpTime;

static int sendPacket(int fd)
{
    unsigned int data[12];
    memset((char *) data, 0, sizeof(data));
    data[0] = htonl((LI << 30) | (VN << 27) | (MODE << 24) | (STRATUM << 16) | (POLL << 8) | (PREC & 0xff));
    data[1] = htonl(1 << 16);
    data[2] = htonl(1 << 16);

    int64_t now = af_gettime_relative();
    data[10] = htonl(now / 1000000 + JAN_1970);
    data[11] = htonl(NTPFRAC(now % 1000000));
    return send(fd, (char *) data, 48, 0);
}
static void getNewTime(unsigned int *data, struct timeval *ptimeval)
{
    NtpTime trantime;
    trantime.coarse = Data(10);
    trantime.fine = Data(11);

    ptimeval->tv_sec = trantime.coarse - JAN_1970;
    ptimeval->tv_usec = USEC(trantime.fine);
}
static int64_t getServerTime(int sockfd, sockaddr_in serv_addr)
{
    fd_set fds;
    struct timeval timeout;
    int ret;
    int count = 0;
    int recv_len;
    unsigned int buf[12];
    memset(buf, 0, sizeof(buf));
    int addr_len;
    addr_len = sizeof(struct sockaddr_in);
    struct timeval TimeSet;
    while (count < 50) {
        FD_ZERO(&fds);
        FD_SET(sockfd, &fds);

        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        ret = select(sockfd + 1, &fds, nullptr, nullptr, &timeout);
        if (ret == 0) {
            count++;
            AF_LOGD("ret == 0\n");
            sendPacket(sockfd);
            af_msleep(100);
            continue;
        }
        if (FD_ISSET(sockfd, &fds)) {
#ifdef WIN32
            recv_len = recvfrom(sockfd, (char*)buf, sizeof(buf), 0, (struct sockaddr *) &serv_addr, &addr_len);
#else
            recv_len = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *) &serv_addr, (socklen_t *) &addr_len);
#endif
            if (recv_len == -1) {
                shutdown(sockfd, 2);
                return -1;
            } else if (recv_len > 0) {
                getNewTime(buf, &TimeSet);
                return TimeSet.tv_sec * 1000000ll + TimeSet.tv_usec;
            }
        } else {
            AF_LOGD("count %d \n", count);
            af_msleep(50);
            count++;
        }
    }
    if (count >= 50) {
        AF_LOGD("getNewTime timeout fail\n");
    }
    return -1;
}
int NTPClient::getNTPTime()
{
    int sockfd, n;

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
    n = sendPacket(sockfd);
    if (n < 0) {
        AF_LOGE("ERROR writing to socket");
        mTime = -errno;
        shutdown(sockfd, 2);
        return -1;
    }
    mTime = getServerTime(sockfd, serv_addr);
    shutdown(sockfd, 2);
    return -1;
}
NTPClient::operator std::string()
{
#define BUFLEN 255
    if (mTime <= 0) {
        return "";
    }
    time_t t = mTime / 1000000;
    char tmpBuf[BUFLEN];
    size_t len = strftime(tmpBuf, BUFLEN, "%Y-%m-%dT%H:%M:%S", gmtime(&t));
    sprintf(tmpBuf + len, ".%03dZ", (int) (mTime % 1000000) / 1000);
    return string(tmpBuf);
}
