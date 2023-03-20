//
// Created by moqi on 2019/10/12.
//

#include "dataSourcePrototype.h"
#include <utils/globalSettings.h>
#ifdef ENABLE_CURL_SOURCE
#include "curl/curl_data_source2.h"
#include "data_source/curl/curl_data_source.h"
#endif
#include "../../plugin/BiDataSource/BiDataSource.h"
#include "ffmpeg_data_source.h"

using namespace Cicada;

dataSourcePrototype *dataSourcePrototype::dataSourceQueue[];
int dataSourcePrototype::_nextSlot;

void dataSourcePrototype::addPrototype(dataSourcePrototype *se)
{
    dataSourceQueue[_nextSlot++] = se;
}

Cicada::IDataSource *dataSourcePrototype::create(const std::string &uri, const Cicada::options *opts, int flags)
{
    int score_res = 0;
    dataSourcePrototype *dataSource = nullptr;
    IDataSource *source = nullptr;

    for (int i = 0; i < _nextSlot; ++i) {
        int score = dataSourceQueue[i]->probeScore(uri, opts, flags);

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
    else if (globalSettings::getSetting().getProperty("protected.network.http.http2") == "ON" && CurlDataSource2::probe(uri)) {
        source = new CurlDataSource2(uri);
    } else if (globalSettings::getSetting().getProperty("protected.network.http.http2") != "ON" && CurlDataSource::probe(uri)) {
        source = new CurlDataSource(uri);
    }
#endif
    else {
        source = new ffmpegDataSource(uri);
    }

    source->setOptions(opts);
    return source;
}
