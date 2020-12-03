//
// Created by moqi on 2019/9/26.
//

#include "codecPrototype.h"

using namespace std;
//vector<codecPrototype *> codecPrototype::codecQueue;
codecPrototype *codecPrototype::codecQueue[];
int codecPrototype::_nextSlot;

void codecPrototype::addPrototype(codecPrototype *se)
{
    codecQueue[_nextSlot++] = se;
    //  AF_LOGD("codecQueue size is %d\n",codecQueue.size());
}

Cicada::IDecoder *codecPrototype::create(const Stream_meta &meta, uint64_t flags, int maxSize,
                                         const Cicada::DrmInfo *drmInfo)
{
    bool bHW = static_cast<bool>(flags & DECFLAG_HW);

    if (bHW) {
        uint64_t decFlags = flags;
        decFlags &= ~DECFLAG_SW;

        for (int i = 0; i < _nextSlot; ++i) {
            if (codecQueue[i]->is_supported(meta, decFlags, maxSize) &&
                (drmInfo == nullptr || codecQueue[i]->is_drmSupport(drmInfo))) {
                return codecQueue[i]->clone();
            }
        }
    }

    for (int i = 0; i < _nextSlot; ++i) {
        if (codecQueue[i]->is_supported(meta, flags, maxSize) &&
            (drmInfo == nullptr || codecQueue[i]->is_drmSupport(drmInfo))) {
            return codecQueue[i]->clone();
        }
    }

    return nullptr;
}
