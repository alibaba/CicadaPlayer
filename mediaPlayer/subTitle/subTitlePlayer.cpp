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
        for (auto item = mSources.begin(); item != mSources.end();) {
            if ((*item)->mSource->getUri() == uri) {
                mListener.onAdded(uri, (*item)->mSource->getID());
                return 0;
            }
            item++;
        }
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
                        (*item)->mNeedFlush++;
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
        /*
         * need flush async
         */
        return true;
        // return mSelectNum > 0 && mEnable;
    }

    void subTitlePlayer::enable(bool bEnable)
    {
        /*
         *  TODO: flush when disable
         */
        mEnable = bEnable;
    }

    int subTitlePlayer::seek(int64_t pts)
    {
        for (auto item = mSources.begin(); item != mSources.end();) {
            if ((*item)->mSelected) {
                (*item)->mSource->seek(std::max(pts + (*item)->mDelay, (int64_t) 0));
                (*item)->mNeedFlush++;
            }

            ++item;
        }

        return 0;
    }

    void subTitlePlayer::render(subTitlePlayer::SourceInfo &info, int64_t pts)
    {
        auto iter = info.mSubtitleShowedQueue.begin();
        while (iter != info.mSubtitleShowedQueue.end()) {
            if ((*iter) && ((*iter)->getInfo().pts + info.mDelay + (*iter)->getInfo().duration) <= pts) {
                mListener.onRender(false, (*iter).release());
                iter = info.mSubtitleShowedQueue.erase(iter);
                continue;
            }
            iter++;
        }

        while (info.mSelected) {
            IAFPacket *packet = nullptr;
            info.getPacket(&packet);

            if (packet && packet->getInfo().pts + info.mDelay <= pts) {

                if (packet->getInfo().pts + info.mDelay + packet->getInfo().duration >= pts) {
                    mListener.onRender(true, packet);
                    info.mSubtitleShowedQueue.push_back(move(info.mPacket));
                } else {
                    AF_LOGD("drop the late subtitle %lld", packet->getInfo().pts);
                    info.mPacket = nullptr;
                }
            } else {
                break;
            }
        }
    }

    void subTitlePlayer::update(int64_t pts)
    {
        for (auto item = mSources.begin(); item != mSources.end();) {
            assert((*item)->mNeedFlush >= 0);
            if ((*item)->mNeedFlush > 0) {
                flushSource((*item).get());
                (*item)->mNeedFlush--;
            }
            if ((*item)->mSelected) {
                render(*(*item), pts);
            }

            ++item;
        }
    }


    void subTitlePlayer::flushSource(SourceInfo *source)
    {
        if (source == nullptr) {
            return;
        }
        while (!source->mSubtitleShowedQueue.empty()) {
            if (source->mSubtitleShowedQueue.front()) {
                mListener.onRender(false, source->mSubtitleShowedQueue.front().release());
            }
            source->mSubtitleShowedQueue.pop_front();
        }
        source->mSubtitleShowedQueue.clear();
        source->mPacket = nullptr;
    }

    void subTitlePlayer::flush()
    {
        for (auto item = mSources.begin(); item != mSources.end();) {
            flushSource((*item).get());
            //mNeedFlush is used for async flush
            //            (*item)->mNeedFlush--;
            ++item;
        }
    }
}// namespace Cicada
