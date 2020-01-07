//
// Created by lifujun on 2019/11/4.
//

#ifndef SOURCE_MD5UTILS_H
#define SOURCE_MD5UTILS_H

#include <string>
namespace Cicada {

    class Md5Utils {
    public:
        static std::string getMd5(const std::string &source);
    };

}
#endif //SOURCE_MD5UTILS_H
