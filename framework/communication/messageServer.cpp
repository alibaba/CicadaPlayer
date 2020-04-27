//
// Created by moqi on 2020/4/22.
//
#include "messageServer.h"
#include "TCPProtocol.h"
#define LOG_TAG "messageServer"
#include <utils/frame_work_log.h>
using namespace Cicada;
messageServer::messageServer(IProtocolServer::Listener *listener)
{
    mServer = static_cast<std::unique_ptr<IProtocolServer>>(new TCPProtocolServer(listener));
}
messageServer::~messageServer()
{}
int messageServer::write(const std::string &msg)
{
    if (!inited) {
        return -1;
    }

    mServer->write_u32(msg.size());
    mServer->write(reinterpret_cast<const uint8_t *>(msg.c_str()), msg.size());
    mServer->flush();
    return 0;
}
int messageServer::write(const std::string &msg, IProtocolServer::IClient *client)
{
    client->write_u32(msg.size());
    client->write(reinterpret_cast<const uint8_t *>(msg.c_str()), msg.size());
    client->flush();
    return 0;
}
int messageServer::init()
{
    if (!inited) {
        int ret = mServer->init();
        if (ret >= 0) {
            inited = true;
            return 0;
        } else
            return -1;
    }
    return 0;
}
std::string messageServer::getServerUri()
{
    return mServer->getServerUri();
}

messageClient::messageClient()
{
    mClient = static_cast<std::unique_ptr<IProtocolClient>>(new TCPProtocolClient());
    mBuffer = static_cast<char *>(malloc(1024));
}
messageClient::~messageClient()
{
    free(mBuffer);
}
int messageClient::connect(const std::string &server)
{
    return mClient->connect(server);
}
std::string messageClient::readMessage()
{
    uint32_t size = 0;
    mClient->read_u32(&size);
    mBuffer[size] = 0;
    auto *p = reinterpret_cast<uint8_t *>(mBuffer);
    while (size > 0) {
        int ret = mClient->read(p, size);
        if (ret > 0) {
            p += ret;
            size -= ret;
        } else {
            break;
        }
    }
    return mBuffer;
}
