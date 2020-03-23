//
// Created by moqi on 2019/10/14.
//

#include "demuxerPrototype.h"
#include <demuxer/avFormatDemuxer.h>
#include <demuxer/avFormatSubtitleDemuxer.h>
#include <demuxer/play_list/playList_demuxer.h>
#include <cassert>

using  namespace Cicada;

demuxerPrototype *demuxerPrototype::demuxerQueue[];
int demuxerPrototype::_nextSlot;

void demuxerPrototype::addPrototype(demuxerPrototype *se)
{
    demuxerQueue[_nextSlot++] = se;
}

IDemuxer *demuxerPrototype::create(const string &uri, const uint8_t *buffer, int64_t size, unique_ptr<Cicada::DemuxerMeta> meta,
                                   const options *opts)
{
    int type = 0;
    DemuxerMeta *demuxerMeta = nullptr;

    if (meta != nullptr) {
        demuxerMeta = meta.get();
    }

    int score_res = 0;
    demuxerPrototype *demuxerType = nullptr;

    for (int i = 0; i < _nextSlot; ++i) {
        int score = demuxerQueue[i]->probeScore(uri, buffer, size, &type, demuxerMeta, opts);

        if (score > score_res) {
            score_res = score;
            demuxerType = demuxerQueue[i];

            if (score >= SUPPORT_MAX) {
                break;
            }
        }
    }

    if (demuxerType && score_res > SUPPORT_NOT) {
        IDemuxer *demuxer = demuxerType->clone(uri, type, demuxerMeta);
        demuxer->setOptions(opts);
        return demuxer;
    }

    return nullptr;
}

Cicada::IDemuxer *demuxerPrototype::create(const string &uri, int type)
{
    // TODO: here not support create a playList demuxer ,because subType is unknown
    for (int i = 0; i < _nextSlot; ++i) {
        if (demuxerQueue[i]->getType() == type) {
            return demuxerQueue[i]->clone(uri, type, nullptr);
        }
    }

    return nullptr;
}

void demuxerPrototype::dummy(int dummy)
{
    if (dummy == 8888) {
#ifdef ENABLE_HLS_DEMUXER
        unique_ptr<IDemuxer> demuxer = unique_ptr<IDemuxer>(new playList_demuxer());
#endif
        unique_ptr<IDemuxer> demuxer1 = unique_ptr<IDemuxer>(new avFormatDemuxer());
        unique_ptr<IDemuxer> demuxer2 = unique_ptr<IDemuxer>(new avFormatSubtitleDemuxer());
        assert(0);
    }
}