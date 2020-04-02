//
// Created by moqi on 2020/4/2.
//

#ifndef CICADAMEDIA_NODE_H
#define CICADAMEDIA_NODE_H

#include <map>
#include <string>
#include <vector>

namespace Cicada {
    namespace xml {
        class Node {
        public:
            Node();

            virtual ~Node();

            void addSubNode(Node *node);

            const std::vector<Node *> &getSubNodes() const;

            void setName(const std::string &name);

            const std::string &getName() const;

            void addAttribute(const std::string &key, const std::string &value);

            bool hasAttribute(const std::string &name) const;

            const std::string &getAttributeValue(const std::string &key) const;

            std::vector<std::string> getAttributeKeys() const;

            void setText(const std::string &text);

            const std::string &getText() const;

            const std::map<std::string, std::string> &getAttributes() const;

            void setType(int type);

            int getType() const;

            std::vector<std::string> toString(int index) const;

        private:
            static const std::string emptyString;
            std::vector<Node *> mSubNodes;
            std::map<std::string, std::string> mAttributes;
            std::string mName;
            std::string mText;
            int mType{-1};
        };
    }// namespace xml
}// namespace Cicada


#endif//CICADAMEDIA_NODE_H
