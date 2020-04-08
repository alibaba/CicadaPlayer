//
// Created by moqi on 2020/2/6.
//

#ifndef CICADAMEDIA_DEMUXERMETAINFO_H
#define CICADAMEDIA_DEMUXERMETAINFO_H

#include <memory>
#include <utils/mediaTypeInternal.h>
#include <vector>

namespace Cicada {
    class DemuxerMetaInfo {
    public:
        uint64_t id;
        std::vector<std::unique_ptr<streamMeta>> meta;
    };
}


#endif //CICADAMEDIA_DEMUXERMETAINFO_H
