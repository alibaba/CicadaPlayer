//
//  AliJSON.cpp
//  AliJSON
//
//  Created by huang_jiafa on 2019/1/2.
//  Copyright (c) 2018 Aliyun. All rights reserved.
//

#include "CicadaJSON.h"
#include "cJSON.h"

using namespace std;

static std::string PrintJson(cJSON *json)
{
    if (nullptr == json) {
        return "";
    }

    const char *strJSON = cJSON_PrintUnformatted(json);

    if (strJSON == nullptr) {
        return "";
    }

    string ret = strJSON;
    cJSON_free((void *)strJSON);
    return ret;
}

CicadaJSONItem::CicadaJSONItem()
{
    mJSON = cJSON_CreateObject();
}

CicadaJSONItem::CicadaJSONItem(const std::string &JSONString)
{
    mJSON = cJSON_Parse(JSONString.c_str());
}

CicadaJSONItem::CicadaJSONItem(const CicadaJSONItem &item)
{
    mJSON = cJSON_Duplicate(item.mJSON, true);
}

bool CicadaJSONItem::isValid()
{
    return (mJSON != nullptr);
}

CicadaJSONItem::CicadaJSONItem(cJSON *JSONOutSide)
{
    mJSON = JSONOutSide;
    mShouldRelease = false;
}

CicadaJSONItem::~CicadaJSONItem()
{
    if (mJSON && mShouldRelease) {
        cJSON_Delete(mJSON);
    }
}

std::string CicadaJSONItem::getString(const std::string &name, const std::string &defaultString) const
{
    if (nullptr == mJSON) {
        return defaultString;
    }

    if (cJSON_HasObjectItem(mJSON, name.c_str())) {
        char *str = cJSON_GetObjectItem(mJSON, name.c_str())->valuestring;

        if (str) {
            return str;
        }
    }

    return defaultString;
}

std::string CicadaJSONItem::getString(const std::string &name) const
{
    return getString(name, "");
}

int CicadaJSONItem::getInt(const std::string &name, int defaultValue) const
{
    if (nullptr == mJSON) {
        return defaultValue;
    }

    if (cJSON_HasObjectItem(mJSON, name.c_str())) {
        return cJSON_GetObjectItem(mJSON, name.c_str())->valueint;
    }

    if (cJSON_HasObjectItem(mJSON, name.c_str())) {
        return cJSON_GetObjectItem(mJSON, name.c_str())->valuedouble;
    }

    return defaultValue;
}

int64_t CicadaJSONItem::getInt64(const std::string &name, int64_t defaultValue) const
{
    return (int64_t)getDouble(name, defaultValue);
}

double CicadaJSONItem::getDouble(const std::string &name, double defaultValue) const
{
    if (nullptr == mJSON) {
        return defaultValue;
    }

    if (cJSON_HasObjectItem(mJSON, name.c_str())) {
        return cJSON_GetObjectItem(mJSON, name.c_str())->valuedouble;
    }

    if (cJSON_HasObjectItem(mJSON, name.c_str())) {
        return cJSON_GetObjectItem(mJSON, name.c_str())->valueint;
    }

    return defaultValue;
}

bool CicadaJSONItem::getBool(const std::string &name, bool defaultValue) const
{
    if (nullptr == mJSON) {
        return defaultValue;
    }

    if (cJSON_HasObjectItem(mJSON, name.c_str())) {
        bool b = cJSON_GetObjectItem(mJSON, name.c_str())->valueint != 0;
        return b;
    }

    return defaultValue;
}

std::string CicadaJSONItem::getStringOfObject(const std::string &name) const
{
    if (nullptr == mJSON) {
        return "";
    }

    if (cJSON_HasObjectItem(mJSON, name.c_str())) {
        cJSON *json = cJSON_GetObjectItem(mJSON, name.c_str());

        if (json != nullptr) {
            return PrintJson(json);
        }
    }

    return "";
}

CicadaJSONItem CicadaJSONItem::getItem(const std::string &name) const
{
    if (nullptr == mJSON) {
        return CicadaJSONItem();
    }

    if (cJSON_HasObjectItem(mJSON, name.c_str())) {
        cJSON *json = cJSON_GetObjectItem(mJSON, name.c_str());

        if (json != nullptr) {
            return CicadaJSONItem(json);
        }
    }

    return CicadaJSONItem();
}

bool CicadaJSONItem::hasItem(const std::string &name) const
{
    if (nullptr == mJSON) {
        return false;
    }

    return cJSON_HasObjectItem(mJSON, name.c_str());
}

void CicadaJSONItem::deleteItem(const std::string &name)
{
    if (nullptr == mJSON) {
        return;
    }

    cJSON_DeleteItemFromObject(mJSON, name.c_str());
}

std::string CicadaJSONItem::printJSON() const
{
    if (mJSON) {
        return PrintJson(mJSON);
    }

    return "";
}

void CicadaJSONItem::addValue(const std::string &name, const char *value)
{
    if (mJSON) {
        cJSON_AddStringToObject(mJSON, name.c_str(), value);
    }
}

void CicadaJSONItem::addValue(const std::string &name, const std::string &value)
{
    if (mJSON) {
        cJSON_AddStringToObject(mJSON, name.c_str(), value.c_str());
    }
}

void CicadaJSONItem::addValue(const std::string &name, long value)
{
    if (mJSON) {
        cJSON_AddNumberToObject(mJSON, name.c_str(), value);
    }
}

void CicadaJSONItem::addValue(const std::string &name, int value)
{
    if (mJSON) {
        cJSON_AddNumberToObject(mJSON, name.c_str(), value);
    }
}

void CicadaJSONItem::addValue(const std::string &name, double value)
{
    if (mJSON) {
        cJSON_AddNumberToObject(mJSON, name.c_str(), value);
    }
}

void CicadaJSONItem::addValue(const std::string &name, bool value)
{
    if (mJSON) {
        cJSON_AddBoolToObject(mJSON, name.c_str(), value);
    }
}


void CicadaJSONItem::addArray(const std::string  &name, CicadaJSONArray &array)
{
    if (mJSON) {
        cJSON_AddItemToObject(mJSON, name.c_str(), array.getJSONCopy());
    }
}


const cJSON *CicadaJSONItem::getJSON()
{
    return mJSON;
}

cJSON *CicadaJSONItem::getJSONCopy() const
{
    return cJSON_Duplicate(mJSON, true);
}


CicadaJSONArray::CicadaJSONArray()
{
    mArray = cJSON_CreateArray();
}

CicadaJSONArray::CicadaJSONArray(const std::string &JSONString)
{
    mArray = cJSON_Parse(JSONString.c_str());
}


CicadaJSONArray::CicadaJSONArray(CicadaJSONItem jsonItem)
{
    mArray = jsonItem.getJSONCopy();
}

CicadaJSONArray::~CicadaJSONArray()
{
    if (mArray) {
        cJSON_Delete(mArray);
    }

    for (auto item : items) {
        delete item;
    }
}

bool CicadaJSONArray::isValid()
{
    return (mArray != nullptr);
}

void CicadaJSONArray::reset()
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (mArray) {
        cJSON_Delete(mArray);
    }

    for (auto item : items) {
        delete item;
    }

    items.clear();
    mArray = cJSON_CreateArray();
}

int CicadaJSONArray::getSize()
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (mArray) {
        return cJSON_GetArraySize(mArray);
    }

    return 0;
}

CicadaJSONItem &CicadaJSONArray::getItem(int index)
{
    std::lock_guard<std::mutex> lock(mMutex);
    static CicadaJSONItem dummy;

    if (nullptr == mArray) {
        return dummy;
    }

    cJSON *itemJSON = cJSON_GetArrayItem(mArray, index);

    if (nullptr == itemJSON) {
        return dummy;
    }

    for (auto item : items) {
        if (item->getJSON() == itemJSON) {
            return *item;
        }
    }

    CicadaJSONItem *item = new CicadaJSONItem(itemJSON);
    items.push_back(item);
    return *item;
}

void CicadaJSONArray::deleteItem(CicadaJSONItem &item)
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (nullptr == mArray) {
        return;
    }

    for (int i = 0; i < cJSON_GetArraySize(mArray); ++i) {
        cJSON *itemJSON = cJSON_GetArrayItem(mArray, i);

        if (item.getJSON() == itemJSON) {
            cJSON_DeleteItemFromArray(mArray, i);
            break;
        }
    }

    std::vector<CicadaJSONItem *>::iterator it;

    for (it = items.begin(); it != items.end(); ++it) {
        if ((*it)->getJSON() == item.getJSON()) {
            delete (*it);
            items.erase(it);
            break;
        }
    }
}

void CicadaJSONArray::addJSON(const CicadaJSONItem &JSON)
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (mArray) {
        cJSON_AddItemToArray(mArray, JSON.getJSONCopy());
    }
}

cJSON *CicadaJSONArray::getJSONCopy() const
{
    return cJSON_Duplicate(mArray, true);
}


//void CicadaJSONArray::addAssignJSON(CicadaJSONItem &JSON)
//{
//    cJSON_AddItemToArray(mArray, JSON.getJSON());
//}

std::string CicadaJSONArray::printJSON() const
{
    std::lock_guard<std::mutex> lock(mMutex);
    return PrintJson(mArray);
}

