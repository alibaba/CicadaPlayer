//
// Created by moqi on 2019/11/21.
//

#ifndef CICADAPLAYERSDK_PLAYEROPTIONS_H
#define CICADAPLAYERSDK_PLAYEROPTIONS_H

#include <base/options.h>

namespace Cicada {
    class playerOptions : public options {
    public:
        playerOptions() = default;

        ~playerOptions() = default;

        int setOpt(const std::string &key, const std::string &value, int64_t flag);

        int setOptPrivate(const std::string &key, const std::string &value, int64_t flag);

        std::string getOpt(const std::string &key) const;

        std::string getOptPrivate(const std::string &key) const;

    private:
        static const std::string privateHeader;
    };
}


#endif //CICADAPLAYERSDK_PLAYEROPTIONS_H
