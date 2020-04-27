//
// Created by moqi on 2020/4/22.
//

#include "TCPProtocol.h"
#define LOG_TAG "TCPProtocol"
extern "C" {
#include <libavutil/bswap.h>
}
#include <utils/frame_work_log.h>
using namespace Cicada;
using namespace std;
InterruptAble::InterruptAble()
{
    mInterruptCB.callback = check_interrupt;
    mInterruptCB.opaque = this;
}
int InterruptAble::check_interrupt(void *pHandle)
{
    auto *server = static_cast<InterruptAble *>(pHandle);
    return server->mInterrupted.load();
}
TCPProtocolServer::TCPProtocolServer(IProtocolServer::Listener *listener) : IProtocolServer(listener)
{}
TCPProtocolServer::~TCPProtocolServer()
{
    interrupt(true);
    mAcceptThread = nullptr;
    avio_close(mServer);
}
std::string TCPProtocolServer::getServerUri()
{
    return "tcp://" + IProtocolServer::getLocalIp() + ":8888";
}
int TCPProtocolServer::init()
{
    AVDictionary *options = nullptr;
    int ret;
    ret = av_dict_set(&options, "listen", "2", 0);
    if (ret < 0) {
        AF_LOGE("set listen error\n");
        return ret;
    }
    if ((ret = avio_open2(&mServer, "tcp://:8888", AVIO_FLAG_WRITE, &mInterruptCB, &options)) < 0) {
        AF_LOGE("Failed to open server: %s\n", av_err2str(ret));
        if (options) {
            av_dict_free(&options);
        }
        return ret;
    }
    if (options) {
        av_dict_free(&options);
    }
    mAcceptThread = unique_ptr<afThread>(NEW_AF_THREAD(accept_loop));
    mAcceptThread->start();
    return 0;
}
int TCPProtocolServer::accept_loop()
{
    int ret;
    AVIOContext *client;
    ret = avio_accept(mServer, &client);
    if (ret >= 0) {
        AF_LOGD("get a client\n");
        auto *pClient = new TCPProtocolClient(client);
        if (mListener) {
            mListener->onAccept(reinterpret_cast<IClient **>(&pClient));
        }
        if (pClient) {
            std::lock_guard<std::mutex> lock(mClientMutex);
            mClients.push_back(static_cast<unique_ptr<TCPProtocolClient>>(pClient));
        }
    }
    return ret;
}
int TCPProtocolServer::write(const uint8_t *buffer, int size)
{
    int ret;
    std::lock_guard<std::mutex> lock(mClientMutex);
    if (mClients.empty()) {
        return 0;
    }
    for (auto &item : mClients) {
        item->write(buffer, size);
    }

    return 0;
}
void TCPProtocolServer::flush()
{
    for (auto &item : mClients) {
        item->flush();
    }
}
int TCPProtocolServer::write_u32(uint32_t val)
{
    int ret;
    std::lock_guard<std::mutex> lock(mClientMutex);
    if (mClients.empty()) {
        return 0;
    }
    for (auto &item : mClients) {
        item->write_u32(val);
    }

    return 0;
}
TCPProtocolClient::TCPProtocolClient() = default;
TCPProtocolClient::~TCPProtocolClient()
{
    interrupt(true);
    avio_close(mClient);
}
int TCPProtocolClient::connect(const string &server)
{
    AVDictionary *format_opts = nullptr;
    av_dict_set_int(&format_opts, "rw_timeout", AV_TIME_BASE / 100, 0);

    int ret = avio_open2(&mClient, server.c_str(), AVIO_FLAG_READ, &mInterruptCB, &format_opts);
    if (format_opts) {
        av_dict_free(&format_opts);
    }
    if (ret < 0) {
        AF_LOGE("connect to server error %s\n", av_err2str(ret));
        avio_close(mClient);
        mClient = nullptr;
    }
    return ret;
}
int TCPProtocolClient::read(uint8_t *buffer, int size)
{
    avio_feof(mClient);
    return avio_read(mClient, buffer, size);
}
int TCPProtocolClient::read_u32(uint32_t *val)
{
    avio_feof(mClient);
    *val = avio_rl32(mClient);
    *val = av_le2ne32(*val);
    return 0;
}
