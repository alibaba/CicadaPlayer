//
// Created by moqi on 2020/6/24.
//
#include "SMP_DCAManager.h"
#include "SuperMediaPlayer.h"
#include <utils/CicadaJSON.h>
#include <cassert>

using namespace std;
using namespace Cicada;
void SMP_DCAObserver::onEvent(int level, const string &content)
{
    CicadaJSONItem item;
    item.addValue("class", mClass);
    item.addValue("obj", to_string((uint64_t) mObj));
    item.addValue("name", mName);
    item.addValue("level", level);
    item.addValue("content", content);
    if (mListener) {
        mListener->onEvent(item.printJSON());
    }
}
void SMP_DCAObserver::setListener(mediaPlayerDCAObserverListener *listener)
{
    mListener = listener;
}
void SMP_DCAObserver::hello()
{
    onEvent(0, "hello");
}
void SMP_DCAManager::createObservers()
{
    if (mDemuxerObserver == nullptr && mPlayer.mDemuxerService && mPlayer.mDemuxerService->getDemuxerHandle()) {
        mDemuxerObserver = static_cast<unique_ptr<SMP_DCAObserver>>(
                new SMP_DCAObserver("demuxer", mPlayer.mDemuxerService->getDemuxerHandle()->getName(), mPlayer.mDemuxerService));
        mDemuxerObserver->setListener(this);
        mDemuxerObserver->hello();
        mPlayer.mDemuxerService->getDemuxerHandle()->setDCAObserver(mDemuxerObserver.get());
    }
}
int SMP_DCAManager::invoke(const string &content)
{
    CicadaJSONItem item(content);
    string ClassName = item.getString("class");
    if (ClassName == "demuxer" && mDemuxerObserver != nullptr) {
        if ((void *) atoll(item.getString("obj").c_str()) == (void *) mPlayer.mDemuxerService) {
            assert(mPlayer.mDemuxerService->getDemuxerHandle());
            if (mPlayer.mDemuxerService->getDemuxerHandle()->getName() == item.getString("name")) {
                return mPlayer.mDemuxerService->getDemuxerHandle()->invoke(item.getInt("cmd", -1), item.getString("content"));
            }
        }
    }
    // TODO: error code
    return 0;
}
void SMP_DCAManager::onEvent(const string &content)
{
    std::lock_guard<std::mutex> guard(mMutex);
    mEventQue.push(content);
}
string SMP_DCAManager::getEvent()
{
    std::lock_guard<std::mutex> guard(mMutex);
    if (!mEventQue.empty()) {
        string event = std::move(mEventQue.front());
        mEventQue.pop();
        return event;
    }
    return string();
}
void SMP_DCAManager::reset()
{
    std::lock_guard<std::mutex> guard(mMutex);
    while (!mEventQue.empty()) {
        mEventQue.pop();
    }
    mDemuxerObserver = nullptr;
}
