//
// Created by moqi on 2020/4/2.
//

#ifndef CICADAMEDIA_DOMPARSER_H
#define CICADAMEDIA_DOMPARSER_H

#include "Node.h"
#include <libxml/xmlreader.h>

namespace Cicada {
    class DOMParser {
    public:
        DOMParser();

        ~DOMParser();

        bool parse(const char *buffer, int size);

        xml::Node *getRootNode();

        void print();

    private:
        xml::Node *processNode();

        void addAttributesToNode(xml::Node *node);

        void print(xml::Node *node, int offset);

    private:
        xmlTextReaderPtr mReader{nullptr};
        xml::Node *mRoot{nullptr};
    };
}// namespace Cicada


#endif//CICADAMEDIA_DOMPARSER_H
