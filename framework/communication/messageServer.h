//
// Created by moqi on 2020/4/22.
//

#ifndef CICADAMEDIA_MESSAGESERVER_H
#define CICADAMEDIA_MESSAGESERVER_H

#include "IProtocol.h"
#include <memory>
#include <string>


namespace Cicada {
    class messageServer {
    public:
        messageServer();
        ~messageServer();

        int init();

        std::string getServerUri();

        int write(const std::string &msg);

    private:
        std::unique_ptr<IProtocolServer> mServer{};
        bool inited{false};
    };

    class messageClient {
    public:
        messageClient();
        ~messageClient();

        int connect(const std::string &server);

        std::string readMessage();

    private:
        std::unique_ptr<IProtocolClient> mClient{};
        char *mBuffer{};
    };
}// namespace Cicada


#endif//CICADAMEDIA_MESSAGESERVER_H
