//
// Created by pingkai on 2021/7/23.
//

#ifndef FRAMEWORK_ISUBTITLERENDER_H
#define FRAMEWORK_ISUBTITLERENDER_H
#include <string>

namespace Cicada {
    class ISubtitleRender {
    public:
        virtual ~ISubtitleRender() = default;
        virtual void setView(void *view) = 0;

        virtual int show(const std::string &data) = 0;

        virtual int hide(const std::string &data) = 0;

        virtual int intHeader(const char *header) = 0;
    };
}// namespace Cicada

#endif//FRAMEWORK_ISUBTITLERENDER_H
