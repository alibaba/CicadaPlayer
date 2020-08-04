//
// Created by moqi on 2019/11/21.
//

#include "options.h"

using namespace std;

#include <cerrno>

std::string Cicada::options::get(const std::string &key) const
{
    auto item = mDict.find(key);
    if (item != mDict.end())
        return (*item).second;
    return "";
}

int Cicada::options::set(const std::string &key, const std::string &value, int64_t flag)
{
    auto item = mDict.find(key);
    if (item != mDict.end()) {
        if (flag == APPEND) {
            mDict[key] += value;
            return 0;
        } else if (flag == REPLACE) {
            mDict[key] = value;
            return 0;
        } else {
            return -EINVAL;
        }
    }
    mDict[key] = value;
    return 0;
}

void Cicada::options::reset()
{
    mDict.clear();
}

std::map<std::string, std::string> Cicada::options::getOptions() {
    return mDict;
}
