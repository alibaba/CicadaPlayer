//
// Created by yuyuan on 2021/03/09.
//

#ifndef DEMUXER_DASH_INHERITABLES_ATTRS_H
#define DEMUXER_DASH_INHERITABLES_ATTRS_H

#include "Timescale.h"
#include <list>
#include <stdint.h>

namespace Cicada {
    namespace Dash {

        class AttrsNode;
        class SegmentTimeline;
        class SegmentTemplate;
        class SegmentList;
        class SegmentBase;

        class AbstractAttr {
        public:
            enum class Type {
                None,
                Playlist,
                SegmentInformation,
                SegmentList,
                SegmentBase,
                SegmentTemplate,
                Timescale,
                Timeline,
                Duration,
                StartNumber,
                AvailabilityTimeOffset,
                AvailabilityTimeComplete,
            };
            AbstractAttr(enum Type);
            virtual ~AbstractAttr();
            AbstractAttr(const AbstractAttr &attr) = delete;
            AbstractAttr &operator=(const AbstractAttr &attr) = delete;
            Type getType() const;
            bool operator==(const AbstractAttr &t) const
            {
                return type == t.getType();
            }
            bool operator!=(const AbstractAttr &t) const
            {
                return type != t.getType();
            }
            virtual bool isValid() const
            {
                return true;
            }
            void setParentNode(AttrsNode *n)
            {
                parentNode = n;
            }

        protected:
            Type type;
            AttrsNode *parentNode;
        };

        class AttrsNode : public AbstractAttr {
        public:
            AttrsNode(Type t, AttrsNode *parent_ = nullptr);
            ~AttrsNode();
            AttrsNode(const AttrsNode &attr) = delete;
            AttrsNode &operator=(const AttrsNode &attr) = delete;
            void addAttribute(AbstractAttr *attr);
            void replaceAttribute(AbstractAttr *attr);
            AbstractAttr *inheritAttribute(AbstractAttr::Type type);
            AbstractAttr *inheritAttribute(AbstractAttr::Type type) const;
            /* helpers */
            uint64_t inheritStartNumber() const;
            int64_t inheritDuration() const;
            Timescale inheritTimescale() const;
            int64_t inheritAvailabilityTimeOffset() const;
            bool inheritAvailabilityTimeComplete() const;
            SegmentTimeline *inheritSegmentTimeline() const;
            SegmentTemplate *inheritSegmentTemplate() const;
            SegmentList *inheritSegmentList() const;
            SegmentBase *inheritSegmentBase() const;

        protected:
            AttrsNode *matchPath(std::list<AbstractAttr::Type> &path);
            AbstractAttr *getAttribute(AbstractAttr::Type type, std::list<AbstractAttr::Type> &path);
            AbstractAttr *getAttribute(AbstractAttr::Type type);
            AbstractAttr *getAttribute(AbstractAttr::Type type) const;
            std::list<AbstractAttr *> props;
            bool is_canonical_root;
        };

        template<enum AbstractAttr::Type e, typename T>
        class AttrWrapper : public AbstractAttr {
        public:
            AttrWrapper(T v) : AbstractAttr(e)
            {
                value = v;
            }
            virtual ~AttrWrapper()
            {}
            AttrWrapper(const AttrWrapper &) = delete;
            AttrWrapper<e, T> &operator=(const AttrWrapper<e, T> &) = delete;
            operator const T &() const
            {
                return value;
            }

        protected:
            T value;
        };

        using AvailabilityTimeOffsetAttr = AttrWrapper<AbstractAttr::Type::AvailabilityTimeOffset, int64_t>;
        using AvailabilityTimeCompleteAttr = AttrWrapper<AbstractAttr::Type::AvailabilityTimeComplete, bool>;
        using StartnumberAttr = AttrWrapper<AbstractAttr::Type::StartNumber, uint64_t>;

        class TimescaleAttr : public AttrWrapper<AbstractAttr::Type::Timescale, Timescale> {
        public:
            TimescaleAttr(Timescale v) : AttrWrapper<AbstractAttr::Type::Timescale, Timescale>(v)
            {}
            virtual bool isValid() const
            {
                return value.isValid();
            }
        };

        class DurationAttr : public AttrWrapper<AbstractAttr::Type::Duration, int64_t> {
        public:
            DurationAttr(int64_t v) : AttrWrapper<AbstractAttr::Type::Duration, int64_t>(v)
            {}
            virtual bool isValid() const
            {
                return value > 0;
            }
        };
    }// namespace Dash
}// namespace Cicada


#endif//DEMUXER_DASH_INHERITABLES_ATTRS_H
