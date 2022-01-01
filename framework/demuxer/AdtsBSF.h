//
// Created by SuperMan on 2020/10/15.
//

#ifndef SOURCE_ADTSBSF_H
#define SOURCE_ADTSBSF_H

extern "C" {
#include <libavformat/avformat.h>
};

#include "AVBSF.h"

namespace Cicada {
    class AdtsBSF : public IAVBSF {
    public:
        AdtsBSF();

        ~AdtsBSF() override;

        int init(const std::string &name, AVCodecParameters *codecpar) override;

        int push(AVPacket *pkt) override;

        int pull(AVPacket *pkt) override;

    private:
        static int io_write(void *opaque, uint8_t *buf, int size);

    private:

        uint8_t *mIobuf = nullptr;
        AVFormatContext *mFormatContext = nullptr;
        AVStream *stream = nullptr;

        AVPacket *targetPkt = nullptr;
    };

}


#endif //SOURCE_ADTSBSF_H
