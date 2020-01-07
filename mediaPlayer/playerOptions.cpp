//
// Created by moqi on 2019/11/21.
//

#include "playerOptions.h"
#include <cerrno>

const std::string Cicada::playerOptions::privateHeader = "internal.";

int Cicada::playerOptions::setOpt(const std::string &key, const std::string &value, int64_t flag)
{
    if (key.size() >= privateHeader.size() &&
        key.substr(0, privateHeader.size()) == privateHeader) {
        return -EPERM;
    }
    return options::set(key, value);
}

std::string Cicada::playerOptions::getOpt(const std::string &key) const
{
    if (key.size() >= privateHeader.size() &&
        key.substr(0, privateHeader.size()) == privateHeader) {
        return "";
    }
    return options::get(key);
}

int Cicada::playerOptions::setOptPrivate(const std::string &key, const std::string &value, int64_t flag)
{
    return options::set(privateHeader + key, value);
}

std::string Cicada::playerOptions::getOptPrivate(const std::string &key) const
{
    return options::get(privateHeader + key);
}
