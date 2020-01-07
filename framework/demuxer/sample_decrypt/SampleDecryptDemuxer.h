//
// Created by moqi on 2019/11/7.
//

#ifndef CICADAPLAYERSDK_SAMPLEDECRYPTDEMUXER_H
#define CICADAPLAYERSDK_SAMPLEDECRYPTDEMUXER_H

#include "ISampleDecryptor.h"
#include <demuxer/avFormatDemuxer.h>

namespace Cicada {
    class SampleDecryptDemuxer : public avFormatDemuxer {

    public:
        explicit SampleDecryptDemuxer();

        void setDecryptor(ISampleDecryptor *decryptor);

        int Open() override;

    private:

        explicit SampleDecryptDemuxer(int dummy);

        Cicada::IDemuxer *clone(const string &uri, int type, const Cicada::DemuxerMeta *meta) override
        {
            return new SampleDecryptDemuxer();
        }

        bool is_supported(const string &uri, const uint8_t *buffer, int64_t size, int *type, const Cicada::DemuxerMeta *meta,
                          const Cicada::options *opts) override
        {
            return false;
        }

        static SampleDecryptDemuxer se;

    private:
        string mKey = "";
        int mCircleCount = 10;

    private:
        ISampleDecryptor *mDecryptor = nullptr;
    };
}


#endif //CICADAPLAYERSDK_SAMPLEDECRYPTDEMUXER_H
