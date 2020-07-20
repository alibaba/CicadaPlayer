//
// Created by moqi on 2020/7/20.
//

#ifndef CICADAMEDIA_CICADAPLAYERPROTOTYPE_H
#define CICADAMEDIA_CICADAPLAYERPROTOTYPE_H
#include "ICicadaPlayer.h"
#include <base/options.h>
#include <base/prototype.h>
#include <utils/CicadaType.h>
namespace Cicada {
    class CICADA_CPLUS_EXTERN CicadaPlayerPrototype {
        static CicadaPlayerPrototype *playerQueue[10];
        static int _nextSlot;

    public:
        virtual ~CicadaPlayerPrototype() = default;

        virtual ICicadaPlayer *clone() = 0;

        virtual bool is_supported(const options *opts) = 0;

        virtual int probeScore(const options *opts)
        {
            if (is_supported(opts)) {
                return SUPPORT_DEFAULT;
            }
            return 0;
        }

        static void addPrototype(CicadaPlayerPrototype *se);

        static ICicadaPlayer *create(const options *opts);
    };
}// namespace Cicada


#endif//CICADAMEDIA_CICADAPLAYERPROTOTYPE_H
