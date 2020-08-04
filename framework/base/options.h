//
// Created by moqi on 2019/11/21.
//

#ifndef CICADAPLAYERSDK_OPTIONS_H
#define CICADAPLAYERSDK_OPTIONS_H

#include <string>
#include <map>
#include <utils/CicadaType.h>

namespace Cicada {
    class CICADA_CPLUS_EXTERN options {
    public:
        static const int APPEND = 1;
        static const int REPLACE = 2;
    public:
        options() = default;

        virtual ~options() = default;

        std::string get(const std::string &key) const;

        int set(const std::string &key, const std::string &value, int64_t flag = 0);

        void reset();

        std::map<std::string, std::string> getOptions();

    private:
        std::map<std::string, std::string> mDict;
    };
}


#endif //CICADAPLAYERSDK_OPTIONS_H
