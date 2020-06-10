//
// Created by moqi on 2018/9/11.
//

#include "af_string.h"

#include <iterator>
#include <regex>

#ifndef __APPLE__
char *strnstr(const char *s, const char *find, size_t slen)
{
    char c, sc;
    size_t len;

    if ((c = *find++) != '\0') {
        len = strlen(find);

        do {
            do {
                if (slen-- < 1 || (sc = *s++) == '\0') {
                    return (NULL);
                }
            } while (sc != c);

            if (len > slen) {
                return (NULL);
            }
        } while (strncmp(s, find, len) != 0);

        s--;
    }

    return ((char *)s);
}
#endif

std::vector<std::string> AfString::s_split(const std::string &in, const std::string &delim)
{
    std::regex re{delim};
    return std::vector<std::string> {
        std::sregex_token_iterator(in.begin(), in.end(), re, -1),
        std::sregex_token_iterator()
    };
}


void AfString::replaceAll(std::string &data, std::string toSearch, std::string replaceStr)
{
    // Get the first occurrence
    size_t pos = data.find(toSearch);

    // Repeat till end is reached
    while (pos != std::string::npos) {
        // Replace this occurrence of Sub String
        data.replace(pos, toSearch.size(), replaceStr);
        // Get the next occurrence from the current position
        pos = data.find(toSearch, pos + replaceStr.size());
    }
}

int af_strstart(const char *str, const char *pfx, const char **ptr)
{
    while (*pfx && *pfx == *str) {
        pfx++;
        str++;
    }

    if (!*pfx && ptr) {
        *ptr = str;
    }

    return !*pfx;
}

size_t af_strlcpy(char *dst, const char *src, size_t size)
{
    size_t len = 0;

    while (++len < size && *src) {
        *dst++ = *src++;
    }

    if (len <= size) {
        *dst = 0;
    }

    return len + strlen(src) - 1;
}

size_t af_strlcat(char *dst, const char *src, size_t size)
{
    size_t len = strlen(dst);

    if (size <= len + 1) {
        return len + strlen(src);
    }

    return len + af_strlcpy(dst + len, src, size - len);
}

void c_make_absolute_url(char *buf, int size, const char *base, const char *rel)
{
    char *sep, *path_query;

    /* Absolute path, relative to the current server */
    if (base && strstr(base, "://") && rel[0] == '/') {
        if (base != buf) {
            af_strlcpy(buf, base, size);
        }

        sep = strstr(buf, "://");

        if (sep) {
            /* Take scheme from base url */
            if (rel[1] == '/') {
                sep[1] = '\0';
            } else {
                /* Take scheme and host from base url */
                sep += 3;
                sep = strchr(sep, '/');

                if (sep) {
                    *sep = '\0';
                }
            }
        }

        af_strlcat(buf, rel, size);
        return;
    }

    /* If rel actually is an absolute url, just copy it */
    if (!base || strstr(rel, "://") || rel[0] == '/') {
        af_strlcpy(buf, rel, size);
        return;
    }
#ifdef WIN32
    if (strnlen_s(rel, 3) >= 3 && isalpha(rel[0]) && rel[1] == ':' && rel[2] == '/') {
        af_strlcpy(buf, rel, size);
        return;
    }
#endif

    if (base != buf) {
        af_strlcpy(buf, base, size);
    }

    /* Strip off any query string from base */
    path_query = strchr(buf, '?');

    if (path_query) {
        *path_query = '\0';
    }

    /* Is relative path just a new query part? */
    if (rel[0] == '?') {
        af_strlcat(buf, rel, size);
        return;
    }

    /* Remove the file name from the base url */
    sep = strrchr(buf, '/');

    if (sep) {
        sep[1] = '\0';
    } else {
        buf[0] = '\0';
    }

    while (af_strstart(rel, "../", NULL) && sep) {
        /* Remove the path delimiter at the end */
        sep[0] = '\0';
        sep = strrchr(buf, '/');

        /* If the next directory name to pop off is "..", break here */
        if (!strcmp(sep ? &sep[1] : buf, "..")) {
            /* Readd the slash we just removed */
            af_strlcat(buf, "/", size);
            break;
        }

        /* Cut off the directory name */
        if (sep) {
            sep[1] = '\0';
        } else {
            buf[0] = '\0';
        }

        rel += 3;
    }

    af_strlcat(buf, rel, size);
}

std::string AfString::make_absolute_url(const std::string &path1, const std::string &path2)
{
    size_t size = path1.length() + path2.length() + 1;
    char *buf = static_cast<char *>(malloc(size));
    c_make_absolute_url(buf, static_cast<int>(size), path1.c_str(), path2.c_str());
    std::string url = buf;
    free(buf);
    return url;
}

bool AfString::isLocalURL(const std::string &url)
{
    if (startWith(url, "file://") || startWith(url, "/")) {
        return true;
    }

#ifdef WIN32
    else if (1 < url.length()
             && ((url[0] == '\\') || (url[1] == ':'))) {
        return true;
    }

#endif
    return false;
}

/* binary search in memory */
int AfString::indexOf(const char *hay, int haysize, const char *needle, int needlesize)
{
    int haypos, needlepos;
    haysize -= needlesize;

    for (haypos = 0; haypos <= haysize; haypos++) {
        for (needlepos = 0; needlepos < needlesize; needlepos++) {
            if (hay[haypos + needlepos] != needle[needlepos]) {
                // Next character in haystack.
                break;
            }
        }

        if (needlepos == needlesize) {
            return haypos;
        }
    }

    return -1;
}

std::map<std::string, std::string> AfString::keyValueToMap(const std::string &source, const std::string &splitStr)
{
    std::map<std::string, std::string> result{};
    std::vector<std::string> subStr = AfString::s_split(source, splitStr);

    for (auto &str : subStr) {
        int equalPos = str.find('=');

        if (equalPos != std::string::npos) {
            const std::string &key = str.substr(0, equalPos);
            const std::string &value = str.substr(equalPos + 1);
            result.insert(std::make_pair(key, value));
        }
    }

    return result;
}

std::vector<uint8_t> AfString::hexSequence(const std::string &value)
{
    std::vector<uint8_t> ret;

    if (value.length() > 2 && (value.substr(0, 2) == "0X" || value.substr(0, 2) == "0x")) {
        for (size_t i = 2; i <= (value.length() - 2); i += 2) {
            unsigned val;
            std::stringstream ss(value.substr(i, 2));
            ss.imbue(std::locale("C"));
            ss >> std::hex >> val;
            ret.push_back(val);
        }
    }

    return ret;
}

