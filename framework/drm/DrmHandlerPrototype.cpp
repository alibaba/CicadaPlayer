
#include "DrmHandlerPrototype.h"

using namespace std;
using namespace Cicada;

DrmHandlerPrototype *DrmHandlerPrototype::drmHandlerQueue[];
int DrmHandlerPrototype::_nextSlot;

void DrmHandlerPrototype::addPrototype(DrmHandlerPrototype *se) {
    drmHandlerQueue[_nextSlot++] = se;
}

DrmHandler *DrmHandlerPrototype::create(const DrmInfo &drmInfo) {

    for (int i = 0; i < _nextSlot; ++i) {
        if (drmHandlerQueue[i]->is_supported(drmInfo)) {
            return drmHandlerQueue[i]->clone(drmInfo);
        }
    }

    return nullptr;
}

bool DrmHandlerPrototype::isSupport(const DrmInfo *drmInfo) {
    if (drmInfo == nullptr) {
        return false;
    }

    for (int i = 0; i < _nextSlot; ++i) {
        if (drmHandlerQueue[i]->is_supported(*drmInfo)) {
            return true;
        }
    }
    return false;
}
