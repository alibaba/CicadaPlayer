//
// Created by lifujun on 2020/5/19.
//

#ifndef SAASMEDIA_URLUTILS_H
#define SAASMEDIA_URLUTILS_H

#include <string>
#include <map>
#include "CicadaType.h"
namespace Cicada {
    class CICADA_CPLUS_EXTERN UrlUtils {
    public:
        static std::map<std::string, std::string> getArgs(const std::string &url);
        static std::string getBaseUrl(const std::string& url);

        static std::string urlEncode( const std::string &c);

    private:
        static std::string char2hex(char dec );
    };
}

#endif //SAASMEDIA_URLUTILS_H
