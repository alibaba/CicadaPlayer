//
// Created by moqi on 2019/10/31.
//

#ifndef CICADAPLAYERSDK_SUBTITLEPLAYER_H
#define CICADAPLAYERSDK_SUBTITLEPLAYER_H

#include <deque>
#include <future>
#include <memory>
#include <string>
#include <vector>

#include "subTitleSource.h"
#include <native_cicada_player_def.h>

namespace Cicada {

    class subTitlePlayer {
    public:
        class Listener {
        public:

            virtual void onRender(bool show, IAFPacket *packet) = 0;

            virtual void onAdded(const std::string &uri, int index) = 0;

            virtual ~Listener() = default;
        };

    private:

        class SourceInfo {

        public:
            explicit SourceInfo(subTitleSource *source, bool selected = false)
            {
                mSource = std::unique_ptr<subTitleSource>(source);
                mSelected = selected;
            }

            int getPacket(IAFPacket **packet)
            {
                if (mPacket == nullptr) {
                    int ret = mSource->read(mPacket);
                    if (mPacket == nullptr) {
                        *packet = nullptr;
                        return ret;
                    }
                }
                *packet = mPacket.get();
                return (*packet)->getSize();
            };

            std::unique_ptr<subTitleSource> mSource;
            bool mSelected{false};
            std::atomic_int mNeedFlush{0};
            std::unique_ptr<IAFPacket> mPacket;
            int64_t mDelay{0};
            std::deque<std::unique_ptr<IAFPacket>> mSubtitleShowedQueue{};
        };


        class Adding {

        public:
            std::future<int> mFuture;
            std::unique_ptr<subTitleSource> mSource;

        };


    public:
        explicit subTitlePlayer(Listener &listener);

        ~subTitlePlayer();

        int add(const std::string &uri);

        void remove(int id);

        int seek(int64_t pts);

        void update(int64_t pts);

        int select(int index, bool bSelect);

        int setDelayTime(int index, int64_t time);

        bool isActive();

        void enable(bool bEnable);

        void onNoop();

        void flush();

    private:
        void render(subTitlePlayer::SourceInfo &info, int64_t pts);
        void flushSource(SourceInfo *source);

    private:
        Listener &mListener;
        std::vector<std::unique_ptr<SourceInfo>> mSources;
        int8_t mSelectNum{0};
        bool mEnable{true};
        std::vector<std::unique_ptr<Adding>> mAddings;

    };
}


#endif //CICADAPLAYERSDK_SUBTITLEPLAYER_H
