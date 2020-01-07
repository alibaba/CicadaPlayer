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

class CICADA_CPLUS_EXTERN dataSourcePrototype {
    static dataSourcePrototype *dataSourceQueue[10];
    static int _nextSlot;
public:

    static Cicada::IDataSource *create(const std::string &uri, const Cicada::options *opts = nullptr);

    virtual ~dataSourcePrototype() = default;

protected:
    static void addPrototype(dataSourcePrototype *se);

private:

    virtual Cicada::IDataSource *clone(const std::string &uri) = 0;

    virtual bool is_supported(const std::string &uri) = 0;

    virtual int probeScore(const std::string &uri, const Cicada::options *opts)
    {
        if (is_supported(uri))
            return Cicada::SUPPORT_DEFAULT;
        return Cicada::SUPPORT_NOT;
    };


};


#endif //CICADA_PLAYER_DATASOURCEPROTOTYPE_H
