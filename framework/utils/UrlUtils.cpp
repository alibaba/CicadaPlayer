//
// Created by lifujun on 2020/5/19.
//

#include "UrlUtils.h"
#include <algorithm>
#include <utils/file/FileUtils.h>

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

void UrlUtils::parseUrl(URLComponents &urlComponents, const std::string &url)
{

    char proto[64] = {0};
    char auth[1024] = {0};
    char host[1024] = {0};
    char path[1024] = {0};
    int port = -1;

    url_split(proto, sizeof(proto), auth, sizeof(auth), host, sizeof(host), &port, path, sizeof(path), url.c_str());

    urlComponents.proto = proto;
    urlComponents.auth = auth;
    urlComponents.host = host;
    urlComponents.path = path;
    urlComponents.port = port;
}


//from ffmpeg/utils.c av_url_split
void UrlUtils::url_split(char *proto, int proto_size, char *authorization, int authorization_size, char *hostname, int hostname_size,
                         int *port_ptr, char *path, int path_size, const char *url)
{
    const char *p, *ls, *at, *at2, *col, *brk;

    if (port_ptr) *port_ptr = -1;
    if (proto_size > 0) proto[0] = 0;
    if (authorization_size > 0) authorization[0] = 0;
    if (hostname_size > 0) hostname[0] = 0;
    if (path_size > 0) path[0] = 0;

    /* parse protocol */
    if ((p = strchr(url, ':'))) {
        strncpy(proto, url, std::min(proto_size, (int) (p + 1 - url)));
        p++; /* skip ':' */
        if (*p == '/') p++;
        if (*p == '/') p++;
    } else {
        /* no protocol means plain filename */
        strncpy(path, url, path_size);
        return;
    }

    /* separate path from hostname */
    ls = p + strcspn(p, "/?#");
    strncpy(path, ls, path_size);

    /* the rest is hostname, use that to parse auth/port */
    if (ls != p) {
        /* authorization (user[:pass]@hostname) */
        at2 = p;
        while ((at = strchr(p, '@')) && at < ls) {
            strncpy(authorization, at2, std::min(authorization_size, (int) (at + 1 - at2)));
            p = at + 1; /* skip '@' */
        }

        if (*p == '[' && (brk = strchr(p, ']')) && brk < ls) {
            /* [host]:port */
            strncpy(hostname, p + 1, std::min(hostname_size, (int) (brk - p)));
            if (brk[1] == ':' && port_ptr) *port_ptr = atoi(brk + 2);
        } else if ((col = strchr(p, ':')) && col < ls) {
            strncpy(hostname, p, std::min((int) (col + 1 - p), hostname_size));
            if (port_ptr) *port_ptr = atoi(col + 1);
        } else
            strncpy(hostname, p, std::min((int) (ls + 1 - p), hostname_size));
    }
}