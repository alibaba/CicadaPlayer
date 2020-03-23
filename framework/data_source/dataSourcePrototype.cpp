//
// Created by moqi on 2019/10/12.
//

#include "dataSourcePrototype.h"
#include "data_source/curl/curl_data_source.h"
#include "../../plugin/BiDataSource.h"
#include "ffmpeg_data_source.h"

using namespace Cicada;

dataSourcePrototype *dataSourcePrototype::dataSourceQueue[];
int dataSourcePrototype::_nextSlot;

void dataSourcePrototype::addPrototype(dataSourcePrototype *se)
{
    dataSourceQueue[_nextSlot++] = se;
}

Cicada::IDataSource *dataSourcePrototype::create(const std::string &uri, const Cicada::options *opts)
{
    int score_res = 0;
    dataSourcePrototype *dataSource = nullptr;
    IDataSource *source = nullptr;

    for (int i = 0; i < _nextSlot; ++i) {
        int score = dataSourceQueue[i]->probeScore(uri, opts);

        if (score > score_res) {
            score_res = score;
            dataSource = dataSourceQueue[i];
        }

        if (score >= SUPPORT_MAX) {
            break;
        }
    }

    if (dataSource) {
        source = dataSource->clone(uri);
    }

#ifdef ENABLE_CURL_SOURCE
    else if (CurlDataSource::probe(uri)) {
        source = new CurlDataSource(uri);
    }

#endif
    else {
        source = new ffmpegDataSource(uri);
    }

    source->setOptions(opts);
    return source;
}
