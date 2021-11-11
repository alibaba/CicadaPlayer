//
// Created by lifujun on 2020/5/19.
//

#ifndef SAASMEDIA_URLUTILS_H
#define SAASMEDIA_URLUTILS_H

#include <string>
#include <map>
#include "CicadaType.h"
namespace Cicada {
    class URLComponents {
    public:
        std::string proto{};
        std::string auth{};
        std::string host{};
        std::string path{};
        int port;
    };

    class CICADA_CPLUS_EXTERN UrlUtils {

    public:
        static std::map<std::string, std::string> getArgs(const std::string &url);
        static std::string getBaseUrl(const std::string& url);

        static void parseUrl(URLComponents &urlComponents, const std::string &url);

        static std::string urlEncode( const std::string &c);

    private:
        static std::string char2hex(char dec );

        static void url_split(char *proto, int proto_size, char *authorization, int authorization_size, char *hostname, int hostname_size,
                              int *port_ptr, char *path, int path_size, const char *url);
    };
}

#endif //SAASMEDIA_URLUTILS_H
