//
// Created by moqi on 2018/4/25.
//

#ifndef FRAMEWORK_HLSTAGS_H
#define FRAMEWORK_HLSTAGS_H

#include <string>
#include <list>
#include <vector>
#include "utils/CicadaType.h"

namespace Cicada{
    namespace hls {
        class Attribute {
        public:
            Attribute(const std::string &, const std::string &);

            Attribute unescapeQuotes() const;

            uint64_t decimal() const;

            std::string quotedString() const;

            double floatingPoint() const;

            std::vector<uint8_t> hexSequence() const;

            std::pair<std::size_t, std::size_t> getByteRange() const;

            std::pair<int, int> getResolution() const;

            std::string name;
            std::string value;
        };

        class CICADA_CPLUS_EXTERN Tag {
        public:
            enum {
                EXTXDISCONTINUITY = 0,
                EXTXENDLIST,
                EXTXIFRAMESONLY,
            };

            Tag(int);

            virtual ~Tag();

            int getType() const;

        private:
            int type;
        };

        class SingleValueTag : public Tag {
        public:
            enum {
                URI = 10,
                EXTXVERSION,
                EXTXBYTERANGE,
                EXTXPROGRAMDATETIME,
                EXTXTARGETDURATION,
                EXTXMEDIASEQUENCE,
                EXTXDISCONTINUITYSEQUENCE,
                EXTXPLAYLISTTYPE,
            };

            SingleValueTag(int, const std::string &);

            virtual ~SingleValueTag();

            const Attribute &getValue() const;

        private:
            Attribute attr;
        };

        class CICADA_CPLUS_EXTERN AttributesTag : public Tag {
        public:
            enum {
                EXTXKEY = 20,
                EXTXMAP,
                EXTXMEDIA,
                EXTXSTREAMINF,
                EXTXPART,
                EXTXPARTINF,
            };

            AttributesTag(int, const std::string &);

            virtual ~AttributesTag();

            const Attribute *getAttributeByName(const char *) const;

            void addAttribute(Attribute *);

        protected:
            virtual void parseAttributes(const std::string &);

            std::list<Attribute *> attributes;
        };


        class CICADA_CPLUS_EXTERN ValuesListTag : public AttributesTag {
        public:
            enum {
                EXTINF = 30
            };

            ValuesListTag(int, const std::string &);

            ~ValuesListTag() override;

        protected:
            void parseAttributes(const std::string &) override;
        };

        class CICADA_CPLUS_EXTERN TagFactory {
        public:
            static Tag *createTagByName(const std::string &, const std::string &);

//            static Attribute *createAttributeByName(const std::string &);
        };
    }
}


#endif //FRAMEWORK_HLSTAGS_H
