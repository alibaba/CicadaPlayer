//
// Created by yuyuan on 2021/03/09.
//
#define LOG_TAG "DOMParser"
#include "DOMHelper.h"
#include <string>
using namespace Cicada::xml;
using namespace Cicada;
using namespace std;

vector<Node *> Cicada::DOMHelper::getElementByTagName(Node *root, const string &name, bool selfContain)
{
    std::vector<Node *> elements;

    for (auto &i : root->getSubNodes()) {
        getElementsByTagName(i, name, &elements, selfContain);
    }

    return elements;
}

vector<Node *> Cicada::DOMHelper::getChildElementByTagName(Node *root, const string &name)
{
    std::vector<Node *> elements;

    for (auto &i : root->getSubNodes()) {
        if (i->getName() == name) elements.push_back(i);
    }

    return elements;
}

Node *Cicada::DOMHelper::getFirstChildElementByName(Node *root, const string &name)
{
    for (auto &i : root->getSubNodes()) {
        if (i->getName() == name) return i;
    }
    return nullptr;
}

void Cicada::DOMHelper::getElementsByTagName(Node *root, const string &name, vector<xml::Node *> *elements, bool selfContain)
{

    if (!selfContain && !root->getName().compare(name)) {
        elements->push_back(root);
        return;
    }

    if (!root->getName().compare(name)) elements->push_back(root);

    for (size_t i = 0; i < root->getSubNodes().size(); i++) {
        getElementsByTagName(root->getSubNodes().at(i), name, elements, selfContain);
    }
}