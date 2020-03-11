//
// Created by lifujun on 2019/11/1.
//

#include "IMuxerPrototype.h"

using namespace Cicada;
using namespace std;

IMuxerPrototype *IMuxerPrototype::muxerPrototypeQueue[];
int IMuxerPrototype::_nextSlot;

IMuxer *IMuxerPrototype::create(const string &destPath, const string &destFormat,
                                const string &description)
{
    int score_res = 0;
    IMuxerPrototype *muxerPrototype = nullptr;

    for (int i = 0; i < _nextSlot; ++i) {
        int score = muxerPrototypeQueue[i]->probeScore(destPath, destFormat, description);

        if (score > score_res) {
            score_res = score;
            muxerPrototype = muxerPrototypeQueue[i];

            if (score >= SUPPORT_MAX) {
                break;
            }
        }
    }

    if (muxerPrototype && score_res > SUPPORT_NOT) {
        return muxerPrototype->clone(destPath, destFormat, description);
    }

    return nullptr;
}

void IMuxerPrototype::addPrototype(IMuxerPrototype *se)
{
    muxerPrototypeQueue[_nextSlot++] = se;
}
