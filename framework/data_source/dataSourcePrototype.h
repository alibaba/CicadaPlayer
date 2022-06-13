//
// Created by moqi on 2019/10/12.
//

#ifndef CICADA_PLAYER_DATASOURCEPROTOTYPE_H
#define CICADA_PLAYER_DATASOURCEPROTOTYPE_H

#include "IDataSource.h"
#include <string>
#include <base/options.h>
#include <base/prototype.h>
#include <utils/CicadaType.h>

const static int DS_NEED_CACHE = 1 << 0;

class CICADA_CPLUS_EXTERN dataSourcePrototype {
    static dataSourcePrototype *dataSourceQueue[10];//TODO: should be replaced with a factory object.
    static int _nextSlot;
public:
    static Cicada::IDataSource *create(const std::string &uri, const Cicada::options *opts = nullptr, int flags = 0);

    virtual ~dataSourcePrototype() = default;

protected:
    static void addPrototype(dataSourcePrototype *se);

private:

    virtual Cicada::IDataSource *clone(const std::string &uri) = 0;

    virtual bool is_supported(const std::string &uri, int flags) = 0;

    virtual int probeScore(const std::string &uri, const Cicada::options *opts, int flags)
    {
        if (is_supported(uri, flags)) return Cicada::SUPPORT_DEFAULT;
        return Cicada::SUPPORT_NOT;
    };


};


#endif //CICADA_PLAYER_DATASOURCEPROTOTYPE_H
