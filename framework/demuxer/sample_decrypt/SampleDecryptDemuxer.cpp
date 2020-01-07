//
// Created by moqi on 2019/11/7.
//

#include "SampleDecryptDemuxer.h"

extern "C" {
#include <libavformat/avformat.h>
extern AVInputFormat sampleDecrypt_demuxer;
};

namespace Cicada {
    SampleDecryptDemuxer SampleDecryptDemuxer::se(0);

    SampleDecryptDemuxer::SampleDecryptDemuxer()
    {
    }

    void SampleDecryptDemuxer::setDecryptor(ISampleDecryptor *decryptor)
    {
        mDecryptor = decryptor;
    }

    int SampleDecryptDemuxer::Open()
    {
        if (mDecryptor) {
            av_dict_set_int(&mInputOpts, "Decryptor", (int64_t) mDecryptor, 0);
        } else {
            return -EINVAL;
        }

        return avFormatDemuxer::open(&sampleDecrypt_demuxer);
    }

    SampleDecryptDemuxer::SampleDecryptDemuxer(int dummy) : avFormatDemuxer(dummy)
    {
        av_register_input_format(&sampleDecrypt_demuxer);
    }
}
