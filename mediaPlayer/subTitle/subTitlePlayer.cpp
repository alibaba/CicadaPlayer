//
// Created by moqi on 2019/10/31.
//

#define LOG_TAG "subTitlePlayer"
#include "subTitlePlayer.h"
#include <cassert>
#include <memory>
#include <utils/frame_work_log.h>

using namespace std;
namespace Cicada {

    subTitlePlayer::subTitlePlayer(Listener &listener)
        : mListener(listener)
    {
    }

    subTitlePlayer::~subTitlePlayer()
    {
        for (auto &item : mAddings) {
            item->mSource->close();
        }

        for (auto &item : mAddings) {
            item->mFuture.wait();
        }
    }

    int subTitlePlayer::add(const std::string &uri)
    {
        auto *adding = new Adding();
        adding->mSource = unique_ptr<subTitleSource>(new subTitleSource(uri));
        subTitleSource *pSource = adding->mSource.get();
        adding->mFuture = std::async(launch::async, [pSource]() {
            return pSource->open();
        });
        mAddings.push_back(unique_ptr<Adding>(adding));
        return 0;
    }

    void subTitlePlayer::onNoop()
    {
        if (mAddings.empty()) {
            return;
        }

        future_status status = mAddings[0]->mFuture.wait_for(chrono::milliseconds(1));

        if (status == future_status::ready) {
            int ret = mAddings[0]->mFuture.get();

            if (ret < 0) {
                mListener.onAdded(mAddings[0]->mSource->getUri(), ret);
            } else {
                mAddings[0]->mSource->setID(mSources.size() | EXT_STREAM_BASE);
                mListener.onAdded(mAddings[0]->mSource->getUri(), mAddings[0]->mSource->getID());
                mSources.push_back(unique_ptr<SourceInfo>(new SourceInfo(mAddings[0]->mSource.release())));
            }

            mAddings.erase(mAddings.begin());
        }
    }

    void subTitlePlayer::remove(int id)
    {
        for (auto item = mSources.begin(); item != mSources.end();) {
            if ((*item)->mSource->getID() == id) {
                item = mSources.erase(item);
                break;
            } else {
                ++item;
            }
        }
    }

    int subTitlePlayer::select(int index, bool bSelect)
    {
        bool find = false;

        for (auto item = mSources.begin(); item != mSources.end();) {
            if ((*item)->mSource->getID() == index) {
                if ((*item)->mSelected != bSelect) {
                    (*item)->mSelected = bSelect;

                    if (bSelect) {
                        ++mSelectNum;
                    } else {
                        --mSelectNum;

                        if ((*item)->mPacket) {
                            IAFPacket *packet = (*item)->mPacket.get();

                            if (packet->getDiscard()) {
                                mListener.onRender(false, (*item)->mPacket.release());
                            }

                            (*item)->mPacket = nullptr;
                        }
                    }
                }

                find = true;
                break;
            }

            ++item;
        }

        assert(mSelectNum >= 0);

        if (find) {
            return 0;
        } else {
            return -EINVAL;
        }
    }

    int subTitlePlayer::setDelayTime(int index, int64_t time)
    {
        bool found = false;
        for (auto item = mSources.begin(); item != mSources.end();) {
            if ((*item)->mSource->getID() == index) {
                (*item)->mDelay = time;
                found = true;
                break;
            }
            ++item;
        }
        if (!found) {
            AF_LOGE("setDelayTime no such stream\n");
        }
        return 0;
    }

    bool subTitlePlayer::isActive()
    {
        return mSelectNum > 0 && mEnable;
    }

    void subTitlePlayer::enable(bool bEnable)
    {
        mEnable = bEnable;
    }

    int subTitlePlayer::seek(int64_t pts)
    {
        for (auto item = mSources.begin(); item != mSources.end();) {
            if ((*item)->mSelected) {
                (*item)->mSource->seek(std::max(pts + (*item)->mDelay, (int64_t) 0));

                if ((*item)->mPacket) {
                    IAFPacket *packet = (*item)->mPacket.get();

                    if (packet->getDiscard()) {
                        mListener.onRender(false, (*item)->mPacket.release());
                    }

                    (*item)->mPacket = nullptr;
                }
            }

            ++item;
        }

        return 0;
    }

    void subTitlePlayer::render(subTitlePlayer::SourceInfo &info, int64_t pts)
    {
        IAFPacket *packet = nullptr;

        do {
            int ret = info.getPacket(&packet);

            if (packet == nullptr) {
                break;
            }

            if (packet->getInfo().pts + info.mDelay + packet->getInfo().duration <= pts) {
                if (packet->getDiscard()) {
                    mListener.onRender(false, info.mPacket.release());
                }

                info.mPacket = nullptr;
            } else {
                break;
            }
        } while (true);

        if (packet == nullptr) {
            return;
        }

        if (packet->getInfo().pts + info.mDelay <= pts) {
            if (!packet->getDiscard()) {
                mListener.onRender(true, packet);
                packet->setDiscard(true);
            }
        }
    }

    void subTitlePlayer::update(int64_t pts)
    {
        for (auto item = mSources.begin(); item != mSources.end();) {
            if ((*item)->mSelected) {
                render(*(*item), pts);
            }

            ++item;
        }
    }

    void subTitlePlayer::flush()
    {
        for (auto item = mSources.begin(); item != mSources.end();) {
            if ((*item)->mPacket && (*item)->mPacket->getDiscard()) {
                mListener.onRender(false, (*item)->mPacket.release());
            }
            (*item)->mPacket = nullptr;
            ++item;
        }
    }
}
