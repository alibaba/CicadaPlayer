//
// Created by SuperMan on 11/27/20.
//

#ifndef SOURCE_DRMINFO_H
#define SOURCE_DRMINFO_H

#include <string>

namespace Cicada {
    class DrmInfo {
    public:
        std::string uri;
        std::string format;

        bool operator==(const DrmInfo &drmInfo) const {
            return uri == drmInfo.uri &&
                   format == drmInfo.format;
        }

        bool empty() const {
            return uri.empty() &&
                   format.empty();
        }

        struct DrmInfoCompare
        {
            bool operator() (const DrmInfo& lhs, const DrmInfo& rhs) const
            {
                return lhs.format < rhs.format || lhs.uri < rhs.uri;
            }
        };
    };
}


#endif //SOURCE_DRMINFO_H
