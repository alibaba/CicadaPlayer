//
// Created by moqi on 2020/4/22.
//

#ifndef CICADAMEDIA_TCPPROTOCOL_H
#define CICADAMEDIA_TCPPROTOCOL_H

#include "IProtocol.h"
#include <queue>
#include <utils/afThread.h>

extern "C" {
#include <libavformat/avio.h>
}
namespace Cicada {
    class InterruptAble {
    public:
        InterruptAble();
        virtual ~InterruptAble() = default;
        void interrupt(bool inter)
        {
            mInterrupted = inter;
        }

    private:
        static int check_interrupt(void *pHandle);

    protected:
        AVIOInterruptCB mInterruptCB{};
        std::atomic_bool mInterrupted{false};
    };
    class TCPProtocolServer : public IProtocolServer, private InterruptAble {
    public:
        class TCPProtocolClient : public IClient {
        public:
            explicit TCPProtocolClient(AVIOContext *context) : mContext(context)
            {}
            ~TCPProtocolClient() override
            {
                avio_close(mContext);
            };

            int write(const uint8_t *buffer, int size) override
            {
                avio_write(mContext, buffer, size);
                return size;
            }

            int write_u32(uint32_t val) override
            {
                avio_wl32(mContext, val);
                return 0;
            }

            void flush() override
            {
                avio_flush(mContext);
            }

        private:
            AVIOContext *mContext;
        };

    public:
        TCPProtocolServer(IProtocolServer::Listener *listener);

        ~TCPProtocolServer() override;

        std::string getServerUri() override;

        int init() override;

        int write(const uint8_t *buffer, int size) override;

        int write_u32(uint32_t val) override;

        void flush() override;

    private:
        int accept_loop();

    private:
        AVIOContext *mServer{nullptr};
        std::mutex mClientMutex{};
        std::vector<std::unique_ptr<TCPProtocolClient>> mClients{};
        std::unique_ptr<afThread> mAcceptThread{};
    };

    class TCPProtocolClient : public IProtocolClient, private InterruptAble {
    public:
        TCPProtocolClient();
        ~TCPProtocolClient() override;
        int connect(const std::string &server) override;
        int read(uint8_t *buffer, int size) override;
        int read_u32(uint32_t *val) override;

    private:
        AVIOContext *mClient{};
    };
}// namespace Cicada


#endif//CICADAMEDIA_TCPPROTOCOL_H
