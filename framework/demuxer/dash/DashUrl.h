//
// Created by yuyuan on 2021/03/16.
//

#ifndef DEMUXER_DASH_DASHURL_H
#define DEMUXER_DASH_DASHURL_H

#include <string>
#include <vector>

namespace Cicada {

    class Representation;

    namespace Dash {

        class SegmentTemplate;

        class DashUrl {
        public:
            class Component {
                friend class DashUrl;

            public:
                Component(const std::string &str, const SegmentTemplate *templ_ = nullptr);

            protected:
                std::string component;
                const SegmentTemplate *templ;

            private:
                bool b_scheme;
                bool b_dir;
                bool b_absolute;
            };

            DashUrl();
            DashUrl(const Component &comp);
            explicit DashUrl(const std::string &str);
            bool hasScheme() const;
            bool empty() const;
            DashUrl &prepend(const Component &comp);
            DashUrl &append(const Component &comp);
            DashUrl &append(const DashUrl &url);
            DashUrl &prepend(const DashUrl &url);
            std::string toString(size_t index, const Representation *rep) const;
            std::string toString() const;

        private:
            std::string UriFixup(const char *str) const;
            std::vector<Component> components;
        };
    }// namespace Dash
}// namespace Cicada


#endif//DEMUXER_DASH_DASHURL_H
