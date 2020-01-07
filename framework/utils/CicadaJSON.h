//
//  CicadaJSON.h
//  CicadaJSON
//
//  Created by huang_jiafa on 2019/1/2.
//  Copyright (c) 2018 Aliyun. All rights reserved.
//

#ifndef CicadaJSON_H
#define CicadaJSON_H

#include <string>
#include <vector>
#include "CicadaType.h"
#include <mutex>

struct cJSON;
class CicadaJSONArray;

class CICADA_CPLUS_EXTERN CicadaJSONItem {

public:
    CicadaJSONItem();
    CicadaJSONItem(const std::string& JSONString);

    CicadaJSONItem(const CicadaJSONItem &);

    bool isValid();

    ~CicadaJSONItem();

    void addValue(const std::string& name, const char* value);
    void addValue(const std::string& name, const std::string& value);
    void addValue(const std::string& name, long value);
    void addValue(const std::string& name, int value);
    void addValue(const std::string& name, double value);
    void addValue(const std::string& name, bool value);
    void addArray(const std::string& name, CicadaJSONArray &array);

    std::string getString(const std::string& name) const;
    std::string getString(const std::string& name, const std::string& defaultString) const;
    int getInt(const std::string& name, int defaultValue) const;
    int64_t getInt64(const std::string& name, int64_t defaultValue) const;
    double getDouble(const std::string& name, double defaultValue) const;
    bool getBool(const std::string& name, bool defaultValue) const;
    CicadaJSONItem getItem(const std::string& name) const;
    std::string getStringOfObject(const std::string& name) const;

    bool hasItem(const std::string& name) const;

    void deleteItem(const std::string& name);

    std::string printJSON() const;


private:
    friend class CicadaJSONArray;
    // init with outside JSON, JSONOutSide will not be deleted in CicadaJSONItemtem
    CicadaJSONItem(cJSON *JSONOutSide);
    
    cJSON *getJSONCopy() const;
    const cJSON *getJSON();

    const CicadaJSONItem &operator=(const CicadaJSONItem &);

private:
    cJSON *mJSON = nullptr;
    bool mShouldRelease = true;
};

class CICADA_CPLUS_EXTERN CicadaJSONArray {

public:
    CicadaJSONArray();
    CicadaJSONArray(const std::string& JSONString);

    CicadaJSONArray(CicadaJSONItem jsonItem);

    ~CicadaJSONArray();

    bool isValid();

    void reset();

    int getSize();

    CicadaJSONItem& getItem(int index);
    void deleteItem(CicadaJSONItem& item);

    // add JSON object by copy
    void addJSON(const CicadaJSONItem &JSON);


    // add JSON object unsafely, better performance, but the JSON param will became invalid.
    //void addAssignJSON(CicadaJSONItem &JSON);

    std::string printJSON() const;
private:
    friend class CicadaJSONItem;
    cJSON *getJSONCopy() const;
    //cJSON *getJSON();

    CicadaJSONArray(const CicadaJSONArray &);
    const CicadaJSONArray &operator=(const CicadaJSONArray &);

private:
    cJSON *mArray = nullptr;
    std::vector<CicadaJSONItem*> items;

    mutable std::mutex mMutex;
};

#endif //AliJSON_H
