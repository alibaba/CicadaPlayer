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
};

using namespace std;

bool CicadaUtils::isEqual(double a, double b)
{
    return (fabs(a - b) < 0.0001f);
}

bool CicadaUtils::startWith(const std::string &src, const std::initializer_list<std::string> &val)
{
    for (const std::string& item : val) {
        if (src.substr(0, item.length()) == item)
            return true;
    }

    return false;
}

vector<string> CicadaUtils::split(const string &str, const char delim)
{
    vector<string> res{};
    if (str.empty())
        return res;

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

string CicadaUtils::base64enc(const string &str)
{

    int out_size = AV_BASE64_SIZE(str.size() + 1);
    string enc;
    char *out = (char *) malloc(out_size);
    char *ret = av_base64_encode(out, out_size, (const uint8_t *) str.c_str(), (int) str.size() + 1);

    if (ret != nullptr) {
        enc = out;
    }
    free(out);
    return enc;
}

string CicadaUtils::base64dec(const string &str)
{
    int out_size = AV_BASE64_DECODE_SIZE(str.size());
    string dec;
    uint8_t *out = (uint8_t *) malloc(out_size);
    int ret = av_base64_decode(out, str.c_str(), out_size);

    if (ret > 0) {
        dec = (char *) out;
    }
    free(out);
    return dec;
}


