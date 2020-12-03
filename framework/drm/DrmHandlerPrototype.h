
#ifndef CICADA_PLAYER_DRMHANDLERPROTOTYPE_H
#define CICADA_PLAYER_DRMHANDLERPROTOTYPE_H

#include <memory>

#include <utils/CicadaType.h>
#include "DrmHandler.h"
#include "DrmInfo.h"

namespace Cicada {

    class CICADA_CPLUS_EXTERN DrmHandlerPrototype {
        static DrmHandlerPrototype *drmHandlerQueue[10];
        static int _nextSlot;
    public:
        virtual ~DrmHandlerPrototype() = default;

        virtual DrmHandler *clone(const DrmInfo &drmInfo) = 0;

        virtual bool is_supported(const DrmInfo &drmInfo) = 0;

        static void addPrototype(DrmHandlerPrototype *se);

        static bool isSupport(const DrmInfo *drmInfo);

        static Cicada::DrmHandler *create(const DrmInfo &drmInfo);
    };
}


#endif //CICADA_PLAYER_DRMHANDLERPROTOTYPE_H
