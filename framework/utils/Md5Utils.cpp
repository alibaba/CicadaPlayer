//
// Created by lifujun on 2019/11/4.
//

#include "Md5Utils.h"

extern "C" {
#include <libavutil/md5.h>
#include <libavutil/mem.h>
}

#include <cstring>

namespace Cicada {

    std::string Md5Utils::getMd5(const std::string &source)
    {
        unsigned char outTmp[256] = {0};
        struct AVMD5 *ctx = av_md5_alloc();
        av_md5_init(ctx);
        av_md5_update(ctx, (const uint8_t *) source.c_str(), source.length());
        av_md5_final(ctx, outTmp);
        av_free(ctx);
        unsigned char out[33] = {0};
        char hex[4] = {0};

        for (int i = 0; i < 16; i++) {
            sprintf(hex, "%02x", outTmp[i]);
            strcat((char *) out, hex);
        }

        return std::string((char *) out);
    }

}