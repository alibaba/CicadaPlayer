//
// Created by yuyuan on 2021/03/16.
//

#ifndef DEMUXER_DASH_URL_TEMPLATE_H
#define DEMUXER_DASH_URL_TEMPLATE_H

#include <string>

namespace Cicada {
    namespace Dash {
        class UrlTemplate {
        public:
            class Token {
            public:
                enum tokentype {
                    TOKEN_ESCAPE,
                    TOKEN_TIME,
                    TOKEN_BANDWIDTH,
                    TOKEN_REPRESENTATION,
                    TOKEN_NUMBER,
                } type;

                std::string::size_type fulllength;
                int width;
            };

            static bool IsDASHToken(const std::string &str, std::string::size_type pos, UrlTemplate::Token &token);

            class TokenReplacement {
            public:
                uint64_t value;
                std::string str;
            };

            static std::string::size_type ReplaceDASHToken(std::string &str, std::string::size_type pos, const UrlTemplate::Token &token,
                                                           const UrlTemplate::TokenReplacement &repl);
        };
    }// namespace Dash
}// namespace Cicada


#endif//DEMUXER_DASH_URL_TEMPLATE_H
