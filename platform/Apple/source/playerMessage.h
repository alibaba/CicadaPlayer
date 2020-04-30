//
// Created by moqi on 2020/4/23.
//

#ifndef CICADAMEDIA_PLAYERMESSAGE_H
#define CICADAMEDIA_PLAYERMESSAGE_H
#include <string>
namespace Cicada {
    class playerMessage {
    public:
        const static std::string prepare;
        const static std::string start;
        const static std::string pause;
        const static std::string exit;
        const static std::string fullScreen;
        const static std::string clock;
        const static std::string seek;
        const static std::string seekAccurate;
    };
}// namespace Cicada


#endif//CICADAMEDIA_PLAYERMESSAGE_H
