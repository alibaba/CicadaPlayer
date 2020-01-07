//
// Created by moqi on 2019-05-29.
//

#ifndef SOURCE_CICADADATASOURCELISTENER_H
#define SOURCE_CICADADATASOURCELISTENER_H

#include <data_source/IDataSource.h>
#include <mutex>

using namespace Cicada;

namespace Cicada {
    class SuperMediaPlayer;

    class SuperMediaPlayerDataSourceListener : public IDataSource::Listener {
    public:
        explicit SuperMediaPlayerDataSourceListener(SuperMediaPlayer &player);

        ~SuperMediaPlayerDataSourceListener() = default;

        NetWorkRetryStatus onNetWorkRetry(int error) override;

        void onNetWorkConnected()override ;

        void enableRetry();

        bool isPending()
        {
            return bWaitingForRet;
        }

        bool isConnected()
        {
            return mNetworkConnected;
        }

    private:
        SuperMediaPlayer &mPlayer;
        NetWorkRetryStatus mStatus = NetWorkRetryStatusRetry;
        bool bWaitingForRet = false;
        int64_t mEffectiveRetryTime = 0;
        std::mutex mMutex;
        std::atomic_bool mNetworkConnected{false};

        void enableRetry_l();
    };
}


#endif //SOURCE_CICADADATASOURCELISTENER_H
