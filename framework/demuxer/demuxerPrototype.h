//
// Created by moqi on 2019/10/14.
//

#ifndef CICADA_PLAYER_DEMUXERPROTOTYPE_H
#define CICADA_PLAYER_DEMUXERPROTOTYPE_H

#include "IDemuxer.h"
#include "utils/CicadaType.h"
#include <base/prototype.h>
#include <base/options.h>
#include "DemuxerMeta.h"


class CICADA_CPLUS_EXTERN demuxerPrototype {
    static demuxerPrototype *demuxerQueue[10];
    static int _nextSlot;
public:
    virtual ~demuxerPrototype() = default;

    virtual Cicada::IDemuxer *
    clone(const string &uri, int type, const Cicada::DemuxerMeta *meta) = 0;

    virtual bool is_supported(const string &uri, const uint8_t *buffer, int64_t size, int *type, const Cicada::DemuxerMeta *meta,
                              const Cicada::options *opts) = 0;


    virtual int probeScore(const string &uri, const uint8_t *buffer, int64_t size, int *type, const Cicada::DemuxerMeta *meta,
                           const Cicada::options *opts)
    {
        if (is_supported(uri, buffer, size, type, meta, opts)){
            return Cicada::SUPPORT_DEFAULT;
        }
        return 0;
    }
    virtual int getType() = 0;

    static void addPrototype(demuxerPrototype *se);

    static Cicada::IDemuxer *
    create(const string &uri, const uint8_t *buffer, int64_t size, unique_ptr<Cicada::DemuxerMeta> meta, const Cicada::options *opts);
    static Cicada::IDemuxer *create(const string &uri, int type);

    static void dummy(int dummy);
};


#endif //CICADA_PLAYER_DEMUXERPROTOTYPE_H
