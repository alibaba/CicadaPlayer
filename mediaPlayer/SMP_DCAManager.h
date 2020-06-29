//
// Created by moqi on 2020/6/24.
//

#ifndef CICADAMEDIA_SMP_DCAMANAGER_H
#define CICADAMEDIA_SMP_DCAMANAGER_H

#include <base/IDCA.h>

#include <memory>
#include <mutex>
#include <queue>

namespace Cicada {
    class SuperMediaPlayer;

    class mediaPlayerDCAObserverListener {
    public:
        virtual void onEvent(const std::string &content) = 0;
    };
    class SMP_DCAObserver : public IDCAObserver {
    public:
        explicit SMP_DCAObserver(std::string className, std::string compName, void *obj)
            : mClass(std::move(className)), mName(compName), mObj(obj)
        {}
        void setListener(mediaPlayerDCAObserverListener *listener);

        void hello();

    private:
        void onEvent(int level, const std::string &content) override;

    private:
        std::string mClass{};
        std::string mName{};
        void *mObj{nullptr};
        mediaPlayerDCAObserverListener *mListener{nullptr};
    };
    class SMP_DCAManager : public mediaPlayerDCAObserverListener {
    public:
        explicit SMP_DCAManager(SuperMediaPlayer &player) : mPlayer(player)
        {}

        void createObservers();

        int invoke(const std::string &content);

        std::string getEvent();

        void reset();

    private:
        void onEvent(const std::string &content) override;

    private:
        SuperMediaPlayer &mPlayer;
        std::unique_ptr<SMP_DCAObserver> mDemuxerObserver{nullptr};
        std::queue<std::string> mEventQue;
        std::mutex mMutex;
    };
}// namespace Cicada


#endif//CICADAMEDIA_SMP_DCAMANAGER_H
