//
// Created by moqi on 2020/4/22.
//

#ifndef CICADAMEDIA_IPROTOCOL_H
#define CICADAMEDIA_IPROTOCOL_H


#include <string>
namespace Cicada {
    class IProtocolServer {
    public:
        class IClient {
        public:
            IClient() = default;

            virtual ~IClient() = default;

            virtual int write(const uint8_t *buffer, int size) = 0;

            virtual int write_u32(uint32_t val) = 0;

            virtual void flush() = 0;

        public:
            std::string ID{};
        };

        class Listener {
        public:
            virtual void onAccept(IClient **client)
            {}
            virtual ~Listener() = default;
        };

    public:
        explicit IProtocolServer(Listener *listener) : mListener(listener)
        {}

        virtual ~IProtocolServer() = default;

        virtual std::string getServerUri() = 0;

        virtual int init() = 0;

        virtual int write(const uint8_t *buffer, int size) = 0;

        virtual int write_u32(uint32_t val) = 0;

        virtual void flush() = 0;
        //    protected:
        static std::string getLocalIp();

    protected:
        Listener *mListener{};
    };

    class IProtocolClient {
    public:
        virtual ~IProtocolClient() = default;

        virtual int connect(const std::string &server) = 0;

        virtual int read(uint8_t *buffer, int size) = 0;

        virtual int read_u32(uint32_t *val) = 0;
    };
}// namespace Cicada


#endif//CICADAMEDIA_IPROTOCOL_H
