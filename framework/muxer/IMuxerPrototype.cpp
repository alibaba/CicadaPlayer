//
// Created by lifujun on 2019/11/1.
//

#include "IMuxerPrototype.h"


IMuxerPrototype *IMuxerPrototype::muxerPrototypeQueue[];
int IMuxerPrototype::_nextSlot;

IMuxer *IMuxerPrototype::create(const string &destPath, const string &destFormat,
                                const string &description)
{
    for (int i = _nextSlot - 1; i >= 0; --i) {
        if (muxerPrototypeQueue[i]->is_supported(destPath, destFormat, description)) {
            return muxerPrototypeQueue[i]->clone(destPath, destFormat, description);
        }
    }

    return nullptr;
}

void IMuxerPrototype::addPrototype(IMuxerPrototype *se)
{
    muxerPrototypeQueue[_nextSlot++] = se;
}
