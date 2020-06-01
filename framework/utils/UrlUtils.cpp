//
// Created by lifujun on 2020/5/19.
//

#include "UrlUtils.h"

using namespace std;
using namespace Cicada;

std::map<std::string, std::string> UrlUtils::getArgs(const std::string &url)
{
    std::map<std::string, std::string> result{};
    size_t paramPos = url.find('?');

    if (paramPos == string::npos) {
        return result;
    }

    const std::string &urlParams = url.substr(paramPos + 1);

    if (urlParams.empty()) {
        return result;
    }

    //copy from https://github.com/vaynedu/Exercise-code/issues/2
    string key;
    string val;
    size_t last_pos = 0;
    size_t amp_pos = 0;  /* & == amp;*/
    size_t equal_pos = 0;

    while ((equal_pos = urlParams.find('=', last_pos)) != string::npos) {
        amp_pos = urlParams.find('&', last_pos);

        if (amp_pos == string::npos) {
            amp_pos = urlParams.length();
        }

        key = urlParams.substr(last_pos, equal_pos - last_pos);
        val = urlParams.substr(equal_pos + 1, amp_pos - (equal_pos + 1));
        result[key] = val;
        last_pos = amp_pos + 1;

        if (last_pos >= urlParams.length()) {
            break;
        }
    }

    return result;
}

std::string UrlUtils::getBaseUrl(const string &url)
{
    size_t paramPos = url.find('?');

    if (paramPos == string::npos) {
        return url;
    }

    return url.substr(0, paramPos);
}

std::string  UrlUtils::char2hex( char dec )
{
    char dig1 = (dec & 0xF0) >> 4;
    char dig2 = (dec & 0x0F);

    if ( 0 <= dig1 && dig1 <= 9) { dig1 += 48; } //0,48 in ascii

    if (10 <= dig1 && dig1 <= 15) { dig1 += 65 - 10; } //A,65 in ascii

    if ( 0 <= dig2 && dig2 <= 9) { dig2 += 48; }

    if (10 <= dig2 && dig2 <= 15) { dig2 += 65 - 10; }

    std::string r;
    r.append( &dig1, 1);
    r.append( &dig2, 1);
    return r;
}

std::string  UrlUtils::urlEncode( const std::string &c )
{
    std::string escaped;
    int max = c.length();

    for (int i = 0; i < max; i++) {
        if ( (48 <= c[i] && c[i] <= 57) ||//0-9
                (65 <= c[i] && c[i] <= 90) ||//ABC...XYZ
                (97 <= c[i] && c[i] <= 122) || //abc...xyz
                (c[i] == '~' || c[i] == '-' || c[i] == '_' || c[i] == '.')
           ) {
            escaped.append( &c[i], 1);
        } else {
            escaped.append("%");
            escaped.append( char2hex(c[i]) );//converts char 255 to string "FF"
        }
    }

    return escaped;
}