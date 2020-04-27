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
        explicit messageServer(IProtocolServer::Listener *listener);
        ~messageServer();

        int init();

        std::string getServerUri();

        int write(const std::string &msg);

        static int write(const std::string &msg, IProtocolServer::IClient *client);

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
