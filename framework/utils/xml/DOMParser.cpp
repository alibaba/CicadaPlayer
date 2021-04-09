//
// Created by moqi on 2020/4/2.
//
#define LOG_TAG "DOMParser"
#include "DOMParser.h"
#include "utils/frame_work_log.h"
#include <cstring>
#include <stack>
#include <string>

using namespace Cicada::xml;

enum NodeType {
    XML_READER_ERROR = -1,
    XML_READER_NONE = 0,
    XML_READER_STARTELEM,
    XML_READER_ENDELEM,
    XML_READER_TEXT,
};


Cicada::DOMParser::DOMParser()
{}

Cicada::DOMParser::~DOMParser()
{
    xmlCleanupParser();
    if (mNodeText) {
        free(mNodeText);
    }
    delete mRoot;
}

bool Cicada::DOMParser::parse(const char *buffer, int size)
{
    mReader = xmlReaderForMemory(buffer, size, nullptr, nullptr, 0);
    if (!mReader) {
        return false;
    }
    mRoot = processNode();
    xmlFreeTextReader(mReader);

    print();

    return mRoot != nullptr;
}

Node *Cicada::DOMParser::processNode()
{
    const char *data;
    int type = XML_READER_NONE;
    std::stack<Node *> lifo;
    while ((type = ReadNextNode(&data)) > 0) {
        switch (type) {
            case XML_READER_STARTELEM: {
                bool empty = xmlTextReaderIsEmptyElement(mReader);
                Node *node = new Node();
                if (node) {
                    if (!lifo.empty()) {
                        lifo.top()->addSubNode(node);
                    }
                    lifo.push(node);

                    node->setName(std::string(data));
                    addAttributesToNode(node);
                }

                if (empty && lifo.size() > 1) {
                    lifo.pop();
                }
                break;
            }

            case XML_READER_TEXT: {
                if (!lifo.empty()) {
                    lifo.top()->setText(std::string(data));
                }
                break;
            }

            case XML_READER_ENDELEM: {
                if (lifo.empty()) {
                    return nullptr;
                }

                Node *node = lifo.top();
                lifo.pop();
                if (lifo.empty()) {
                    return node;
                }
            }

            default:
                break;
        }
    }

    while (lifo.size() > 1) {
        lifo.pop();
    }
    Node *node = (!lifo.empty()) ? lifo.top() : nullptr;
    return node;
}

void Cicada::DOMParser::addAttributesToNode(Node *node)
{
    //if (xmlTextReaderHasAttributes(mReader)) {
    //    while (xmlTextReaderMoveToNextAttribute(mReader)) {
    //        std::string key = (const char *) xmlTextReaderConstName(mReader);
    //        std::string value = (const char *) xmlTextReaderConstValue(mReader);
    //        node->addAttribute(key, value);
    //    }
    //}
    const char *attrValue;
    const char *attrName;
    while ((attrName = ReadNextAttr(&attrValue)) != nullptr) {
        if (attrValue) {
            std::string key = attrName;
            std::string value = attrValue;
            node->addAttribute(key, value);
        }
    }
}

Node *Cicada::DOMParser::getRootNode()
{
    return mRoot;
}

void Cicada::DOMParser::print()
{
    print(mRoot, 0);
}

void Cicada::DOMParser::print(Node *node, int offset)
{
    std::string ss;
    for (int i = 0; i < offset; i++) ss += " ";

    ss += node->getName();

    std::vector<std::string> keys = node->getAttributeKeys();

    for (auto &key : keys) {
        ss += " ";
        ss += key;
        ss += "=";
        ss += node->getAttributeValue(key);
    }

    AF_LOGD("%s\n", ss.c_str());

    offset++;

    for (auto i : node->getSubNodes()) {
        this->print(i, offset);
    }
}

int Cicada::DOMParser::ReadNextNode(const char **data)
{
    const xmlChar *node = nullptr;
    int ret = XML_READER_NONE;

    if (mNodeText) {
        free(mNodeText);
        mNodeText = nullptr;
    }
    int readerRet = 0;
    int retryCount = 0;
skip:
    readerRet = xmlTextReaderRead(mReader);
    switch (readerRet) {
        case 0: /* EOF */
            return XML_READER_NONE;
        case -1: /* error */
        {
            ++retryCount;
            if (retryCount >= 5) {
                return XML_READER_ERROR;
            } else {
                goto skip;
            }
        }
    }
    readerRet = xmlTextReaderNodeType(mReader);
    switch (readerRet) {
        case XML_READER_TYPE_ELEMENT:
            node = xmlTextReaderConstName(mReader);
            ret = XML_READER_STARTELEM;
            break;

        case XML_READER_TYPE_END_ELEMENT:
            node = xmlTextReaderConstName(mReader);
            ret = XML_READER_ENDELEM;
            break;

        case XML_READER_TYPE_CDATA:
        case XML_READER_TYPE_TEXT:
            node = xmlTextReaderConstValue(mReader);
            ret = XML_READER_TEXT;
            break;

        case -1:
            return XML_READER_ERROR;

        default:
            goto skip;
    }

    if (node == nullptr) {
        return XML_READER_ERROR;
    }

    mNodeText = strdup((const char *) node);
    if (data != NULL) {
        *data = mNodeText;
    }
    return mNodeText != NULL ? ret : XML_READER_ERROR;
}

const char *Cicada::DOMParser::ReadNextAttr(const char **data)
{
    const xmlChar *name, *value;

    if (xmlTextReaderMoveToNextAttribute(mReader) != 1 || (name = xmlTextReaderConstName(mReader)) == NULL ||
        (value = xmlTextReaderConstValue(mReader)) == NULL)
        return NULL;

    *data = (const char *) value;
    return (const char *) name;
}