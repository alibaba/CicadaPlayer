//
// Created by yuyuan on 2021/03/08.
//

#ifndef DEMUXER_DASH_PROGRAMINFO_H
#define DEMUXER_DASH_PROGRAMINFO_H
#include <string>

namespace Cicada {
    namespace Dash {
        class ProgramInfo {
        public:
            std::string moreInformationUrl{};
            std::string title{};
            std::string source{};
            std::string copyright{};
        };
    }// namespace Dash
}// namespace Cicada


#endif//CICADAMEDIA_PROGRAMINFO_H
