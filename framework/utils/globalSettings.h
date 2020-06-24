//
// Created by lifujun on 2019/4/11.
//

#ifndef SOURCE_GLOBALSETTINGS_H
#define SOURCE_GLOBALSETTINGS_H


#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <map>
#include <set>
#include "af_string.h"

using namespace std;

namespace Cicada{
    class globalSettings {
    private:
        class property {
        public:
            string     key   = "";
            string     value = "";
            thread::id bindingTid;
        };
        globalSettings() = default;

    public:
        using type_resolve = std::map<std::string, std::set<std::string>>;
        static globalSettings *getSetting();

        int setProperty(const string &key, const string &value);

        const string &getProperty(const string &key);

        int addResolve(const string &host, const string &ip);

        void removeResolve(const string &host, const string &ip);

        mutex &getMutex()
        {
            return mMutex;
        }

        const type_resolve &getResolve()
        {
            return mResolve;
        }

        void setIpResolveType(int value) {
            setProperty("protected.IpResolveType", AfString::to_string(value));
        }

        int getIpResolveType() {
            const string& value = getProperty("protected.IpResolveType");
            return value.empty() ? 0 /*IpResolveWhatEver*/ : atoi(value.c_str());
        }

    private:
        mutex             mMutex;
        vector <property> mProperties;
        const string      mDefaultKeyValue = "";
        type_resolve mResolve;
    };
};



#endif //SOURCE_GLOBALSETTINGS_H
