//
// Created by moqi on 2020/4/2.
//

#include "Node.h"

using namespace Cicada::xml;
using namespace std;

const string Node::emptyString;

Node::Node() = default;

Node::~Node()
{
    for (auto &mSubNode : mSubNodes) {
        delete mSubNode;
    }
}

void Node::addSubNode(Cicada::xml::Node *node)
{
    mSubNodes.push_back(node);
}

const vector<Node *> &Node::getSubNodes() const
{
    return mSubNodes;
}
void Node::setName(const string &name)
{
    mName = name;
}
const string &Node::getName() const
{
    return mName;
}
void Node::addAttribute(const string &key, const string &value)
{
    mAttributes[key] = value;
}
bool Node::hasAttribute(const string &name) const
{
    return mAttributes.find(name) != mAttributes.end();
}
const string &Node::getAttributeValue(const string &key) const
{
    auto it = mAttributes.find(key);

    if (it != mAttributes.end()) return it->second;
    return emptyString;
}
vector<std::string> Node::getAttributeKeys() const
{
    vector<std::string> keys;
    map<string, string>::const_iterator it;

    for (it = mAttributes.begin(); it != mAttributes.end(); ++it) {
        keys.push_back(it->first);
    }
    return keys;
}
void Node::setText(const string &text)
{
    mText = text;
}
const string &Node::getText() const
{
    return mText;
}
const map<string, string> &Node::getAttributes() const
{
    return mAttributes;
}
void Node::setType(int type)
{
    mType = type;
}
int Node::getType() const
{
    return mType;
}

std::vector<std::string> Node::toString(int index) const
{
    std::vector<std::string> ret;
    std::string text(index, ' ');
    text.append(getName());
    ret.push_back(text);
    for (auto item = mSubNodes.begin(); item < mSubNodes.end(); ++item) {
        std::vector<std::string> sub = (*item)->toString(index + 1);
        ret.insert(ret.end(), sub.begin(), sub.end());
    }
    return ret;
}
