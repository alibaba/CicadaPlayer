//
// Created by moqi on 2020/4/2.
//
#define LOG_TAG "DOMParser"
#include "DOMParser.h"
#include "../frame_work_log.h"
#include <stack>
#include <string>
using namespace Cicada::xml;
using namespace std;


enum NodeType {
    XML_READER_STARTELEM = XML_READER_TYPE_ELEMENT,
    XML_READER_ENDELEM = XML_READER_TYPE_END_ELEMENT,
    XML_READER_COMMENT = XML_READER_TYPE_COMMENT,
    XML_READER_WHITESPACE = XML_READER_TYPE_SIGNIFICANT_WHITESPACE,
    XML_READER_TEXT = XML_READER_TYPE_TEXT,
};


Cicada::DOMParser::DOMParser()
{}

Cicada::DOMParser::~DOMParser()
{
    xmlCleanupParser();
    delete mRoot;
}

bool Cicada::DOMParser::parse(const char *buffer, int size)
{
    mReader = xmlReaderForMemory(buffer, size, nullptr, nullptr, 0);
    if (!mReader) {
        return false;
    }
    if (xmlTextReaderRead(mReader)) {
        mRoot = processNode();
    }
    xmlFreeTextReader(mReader);

    return mRoot != nullptr;
}
Node *Cicada::DOMParser::processNode()
{
    int type = xmlTextReaderNodeType(mReader);

    if (type == XML_READER_TEXT) {
        const char *text = (const char *) xmlTextReaderReadString(mReader);

        if (text != nullptr) {
            Node *node = new Node();
            node->setType(type);
            node->setText(text);
            delete text;
            return node;
        } else
            return nullptr;
    } else if (type == XML_READER_WHITESPACE) {
        return nullptr;
    }

    while (type == XML_READER_COMMENT || type == XML_READER_WHITESPACE) {
        xmlTextReaderRead(mReader);
        type = xmlTextReaderNodeType(mReader);
    }
    Node *node = new Node();
    node->setType(type);

    if (xmlTextReaderConstName(mReader) == nullptr) {
        delete node;
        return nullptr;
    }
    std::string name = (const char *) xmlTextReaderConstName(mReader);
    int isEmpty = xmlTextReaderIsEmptyElement(mReader);
    node->setName(name);
    this->addAttributesToNode(node);

    if (isEmpty) {
        return node;
    }
    Node *subNode;
    int ret;

    do {
        ret = xmlTextReaderRead(mReader);
        if (ret != 1) {
            break;
        }
        if (!strcmp(name.c_str(), (const char *) xmlTextReaderConstName(mReader))) {
            return node;
        }

        subNode = this->processNode();

        if (subNode != nullptr) {
            node->addSubNode(subNode);
        }
    } while (true);

    return node;
}
void Cicada::DOMParser::addAttributesToNode(Node *node)
{
    if (xmlTextReaderHasAttributes(mReader)) {
        while (xmlTextReaderMoveToNextAttribute(mReader)) {
            std::string key = (const char *) xmlTextReaderConstName(mReader);
            std::string value = (const char *) xmlTextReaderConstValue(mReader);
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
    string ss;
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
