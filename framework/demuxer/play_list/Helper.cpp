//
// Created by moqi on 2018/4/26.
//

#include "Helper.h"
#include <algorithm>
#include <utils/af_string.h>

namespace Cicada {

    std::string Helper::combinePaths(const std::string &path1, const std::string &path2)
    {
        return AfString::make_absolute_url(path1, path2);
    }

    std::string Helper::getDirectoryPath(const std::string &path)
    {
        std::size_t pos = path.find_last_of('/');
        return (pos != std::string::npos) ? path.substr(0, pos) : path;
    }

    std::string Helper::getFileExtension(const std::string &uri)
    {
        std::string extension;
        std::size_t pos = uri.find_first_of("?#");

        if (pos != std::string::npos) {
            extension = uri.substr(0, pos);
        } else {
            extension = uri;
        }

        pos = extension.find_last_of('.');

        if (pos == std::string::npos || extension.length() - pos < 2) {
            return std::string();
        }

        return extension.substr(pos + 1);
    }

    bool Helper::ifind(std::string haystack, std::string needle)
    {
        transform(haystack.begin(), haystack.end(), haystack.begin(), toupper);
        transform(needle.begin(), needle.end(), needle.begin(), toupper);
        return haystack.find(needle) != std::string::npos;
    }

    std::list<std::string> Helper::tokenize(const std::string &str, char c)
    {
        std::list<std::string> ret;
        std::size_t prev = 0;
        std::size_t cur = str.find_first_of(c, 0);

        while (cur != std::string::npos) {
            ret.push_back(str.substr(prev, cur - prev));
            prev = cur + 1;
            cur = str.find_first_of(c, cur + 1);
        }

        ret.push_back(str.substr(prev));
        return ret;
    }

}
