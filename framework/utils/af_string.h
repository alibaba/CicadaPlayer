//
// Created by moqi on 2018/9/11.
//

#ifndef SOURCE_AF_STRING_H
#define SOURCE_AF_STRING_H

#include <string>
#include <vector>
#include <sstream>
#include "CicadaType.h"

#ifndef __APPLE__
#ifdef __cplusplus
extern "C" {
#endif
char *strnstr(const char *s, const char *find, size_t slen);
#ifdef __cplusplus
}
#endif
#endif

class CICADA_CPLUS_EXTERN AfString {
public:
    static inline int isSpace(int c)
    {
        return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
    }

    static void trimString(std::string &str)
    {
        std::size_t n = str.find_last_not_of(" \r\n\t");
        if (n != std::string::npos) {
            str.erase(n + 1, str.size() - n);
        }
        n = str.find_first_not_of(" \r\n\t");
        if (n != std::string::npos) {
            str.erase(0, n);
        }

        return;
    }

    static std::vector<std::string> s_split(const std::string &in, const std::string &delim);

    template<typename T>
    static std::string to_string(T value)
    {
#ifdef ANDROID
        std::ostringstream os;
        os << value;
        return os.str();
#else
        return std::to_string(value);
#endif
    }

    static void replaceAll(std::string &data, std::string toSearch, std::string replaceStr);

    static std::string make_absolute_url(const std::string &path1, const std::string &path2);

    static bool isLocalURL(const std::string &url);

    static inline bool startWith(const std::string &str, const std::string &head)
    {
        return str.compare(0, head.size(), head) == 0;
    }

    static inline bool endWith(const std::string &str, const std::string &tail)
    {
        return str.compare(str.size() - tail.size(), tail.size(), tail) == 0;
    }
};

#endif //SOURCE_AF_STRING_H
