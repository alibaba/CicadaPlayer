//
// Created by lifujun on 2019/11/4.
//

#include "Md5Utils.h"

#include <openssl/md5.h>
#include <cstring>

namespace Cicada {

    std::string Md5Utils::getMd5(const std::string &source) {
        unsigned char outTmp[256] = {0};
        MD5_CTX ctx;
        MD5_Init(&ctx);
        MD5_Update(&ctx, source.c_str(), source.length());
        MD5_Final(outTmp, &ctx);
        unsigned char out[33] = {0};
        char hex[4] = {0};

        for (int i = 0; i < 16; i++) {
            sprintf(hex, "%02x", outTmp[i]);
            strcat((char *) out, hex);
        }

        return std::string((char *) out);
    }

}