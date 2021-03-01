//
//  CicadaUtils.cpp
//  framework_utils
//
//  Created by jeff on 2019/4/10.
//

#include "CicadaUtils.h"

#include <math.h>

extern "C" {
#include <libavutil/base64.h>
#include <libavutil/hmac.h>
}

using namespace std;

bool CicadaUtils::isEqual(double a, double b)
{
    return (fabs(a - b) < 0.0001f);
}

bool CicadaUtils::startWith(const std::string &src, const std::initializer_list<std::string> &val)
{
    for (const std::string &item : val) {
        if (src.substr(0, item.length()) == item) {
            return true;
        }
    }

    return false;
}

vector<string> CicadaUtils::split(const string &str, const char delim)
{
    vector<string> res{};

    if (str.empty()) {
        return res;
    }

    const char *c_str = str.c_str();
    size_t startPos = 0;
    size_t endPos = 0;
    size_t len = str.length();

    while (endPos < len) {
        if (c_str[endPos] != delim) {
            endPos++;
        } else {
            res.push_back(str.substr(startPos, endPos - startPos));
            endPos++;
            startPos = endPos;
        }
    }

    if (endPos > startPos) {
        res.push_back(str.substr(startPos, endPos));
    }

    return res;
}

std::string CicadaUtils::base64enc(const char *str, int len)
{
    int out_size = AV_BASE64_SIZE(len);
    string enc;
    char *out = (char *) malloc(out_size);
    char *ret = av_base64_encode(out, out_size, (const uint8_t *) str, len);

    if (ret != nullptr) {
        enc = out;
    }

    free(out);
    return enc;
}


string CicadaUtils::base64enc(const string &str)
{
    return base64enc(str.c_str(), str.size() + 1);
}

string CicadaUtils::base64dec(const string &str)
{
    string dec;
    char *out = nullptr;
    int ret = base64dec(str, &out);

    if (ret > 0) {
        dec = out;
    }

    free(out);
    return dec;
}

int CicadaUtils::base64dec(const string &str, char **dst)
{
    int out_size = AV_BASE64_DECODE_SIZE(str.size());
    uint8_t *out = (uint8_t *) malloc(out_size + 1);
    int ret = av_base64_decode(out, str.c_str(), out_size);

    if (ret > 0) {
        out[ret] = 0;
        *dst = reinterpret_cast<char *>(out);
        return ret;
    } else {
        free(out);
        return -1;
    }
}

int CicadaUtils::hmac_sha1(uint8_t **dst, const uint8_t *data, unsigned int dataLen, const uint8_t *key, unsigned int keyLen)
{
    if (data == nullptr || dataLen <= 0 || key == nullptr || keyLen <= 0) {
        return -1;
    }

    AVHMAC *shaContext = av_hmac_alloc(AV_HMAC_SHA1);
    av_hmac_init(shaContext, key, keyLen);
    av_hmac_update(shaContext, data, dataLen);

    *dst = static_cast<uint8_t *>(malloc(20));
    int ret = av_hmac_final(shaContext, *dst, 20);
    av_hmac_free(shaContext);
    return ret;
}
