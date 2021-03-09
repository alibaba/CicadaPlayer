//
// Created by yuyuan on 2021/03/16.
//

#ifndef DEMUXER_DASH_IDASHURL_H
#define DEMUXER_DASH_IDASHURL_H

#include "DashUrl.h"

namespace Cicada {
    namespace Dash {
        class IDashUrl {
        public:
            IDashUrl(const IDashUrl *parent = nullptr)
            {
                setParent(parent);
            }
            virtual ~IDashUrl() = default;
            virtual DashUrl getUrlSegment() const = 0;
            void setParent(const IDashUrl *parent)
            {
                parentUrlMember = parent;
            }

        protected:
            DashUrl getParentUrlSegment() const
            {
                return (parentUrlMember) ? parentUrlMember->getUrlSegment() : DashUrl();
            }

        private:
            const IDashUrl *parentUrlMember;
        };
    }// namespace Dash
}// namespace Cicada


#endif//DEMUXER_DASH_IDASHURL_H
