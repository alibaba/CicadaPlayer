//
// Created by moqi on 2018/11/27.
//

#include "property.h"
#include "globalSettings.h"

using namespace Cicada;


const char *getProperty(const char *key)
{
    return globalSettings::getSetting()->getProperty(key).c_str();
}

int setProperty(const char *key, const char *value)
{
    return globalSettings::getSetting()->setProperty(key, value);
}
