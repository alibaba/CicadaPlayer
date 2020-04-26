//
// Created by moqi on 2020/4/22.
//

#include <communication/IProtocol.h>
#include <communication/TCPProtocol.h>
#include <communication/messageServer.h>
#include <utils/frame_work_log.h>
#include <utils/timer.h>
using namespace Cicada;
using namespace std;
static void testProtocol()
{
    //    string ip = IProtocolServer::getLocalIp();
    //    AF_LOGD("ip is %s\n", ip.c_str());
    TCPProtocolServer server;
    server.init();
    TCPProtocolClient client;
    client.connect(server.getServerUri());
    TCPProtocolClient client1;
    client1.connect(server.getServerUri());
    af_msleep(10);
    uint8_t buffer[10] = "hello";
    server.write(reinterpret_cast<const uint8_t *>("hello"), 6);
    memset(buffer, 0, 10);
    client.read(buffer, 10);
    AF_LOGD("buffer is %s\n", buffer);

    memset(buffer, 0, 10);
    client1.read(buffer, 10);
    AF_LOGD("buffer is %s\n", buffer);
}

static void testMessage()
{
    messageServer server;
    messageClient client[1]{};

    server.init();
    string serverUrl = server.getServerUri();
    for (auto &item : client) {
        item.connect(serverUrl);
    }

    af_msleep(10);
    for (auto &item : client) {
        AF_LOGD("client receive %s\n", item.readMessage().c_str());
    }

    server.write("12345");
    af_msleep(1000);
    for (auto &item : client) {
        AF_LOGD("client receive %s\n", item.readMessage().c_str());
    }
}
int main()
{
    //   testProtocol();
    testMessage();

    return 0;
}
