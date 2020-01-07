//
// Created by moqi on 2018/4/26.
//

#ifndef FRAMEWORK_HELPER_H
#define FRAMEWORK_HELPER_H

#include <string>
#include <list>

namespace Cicada{
    class Helper {
    public:
        static std::string combinePaths(const std::string &path1, const std::string &path2);

        static std::string getDirectoryPath(const std::string &path);

        static std::string getFileExtension(const std::string &uri);

        static bool ifind(std::string haystack, std::string needle);

        static std::list<std::string> tokenize(const std::string &, char);
    };
}


#endif //FRAMEWORK_HELPER_H
