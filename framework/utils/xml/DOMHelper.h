//
// Created by yuyuan on 2021/03/09.
//

#ifndef FRAMEWORK_UTILS_DOMHELPER_H
#define FRAMEWORK_UTILS_DOMHELPER_H

#include "Node.h"

namespace Cicada {
    class DOMHelper {
    public:
        static std::vector<xml::Node *> getElementByTagName(xml::Node *root, const std::string &name, bool selfContain);
        static std::vector<xml::Node *> getChildElementByTagName(xml::Node *root, const std::string &name);
        static xml::Node *getFirstChildElementByName(xml::Node *root, const std::string &name);

    private:
        static void getElementsByTagName(xml::Node *root, const std::string &name, std::vector<xml::Node *> *elements, bool selfContain);
    };
}// namespace Cicada


#endif// FRAMEWORK_UTILS_DOMHELPER_H
