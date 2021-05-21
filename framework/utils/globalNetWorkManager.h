//
// Created by 平凯 on 2021/5/17.
//

#ifndef CICADAMEDIA_GLOBALNETWORKMANAGER_H
#define CICADAMEDIA_GLOBALNETWORKMANAGER_H
#include <mutex>
#include <set>

namespace Cicada {
    class globalNetWorkManager {
    public:
        class globalNetWorkManagerListener {

            ;

        public:
            virtual void OnReconnect()
            {}
        };

    public:
        static globalNetWorkManager *getGlobalNetWorkManager();
        void addListener(globalNetWorkManagerListener *listener);
        void removeListener(globalNetWorkManagerListener *listener);
        void reConnect();


    private:
        globalNetWorkManager() = default;
        ~globalNetWorkManager() = default;

    private:
        std::mutex mMutex;
        std::set<globalNetWorkManagerListener *> mListeners{};
    };
}// namespace Cicada


#endif//CICADAMEDIA_GLOBALNETWORKMANAGER_H
