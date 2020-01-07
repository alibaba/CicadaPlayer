//
// Created by lifujun on 2019/3/14.
//

#ifndef SOURCE_ERRORCODEMAP_H
#define SOURCE_ERRORCODEMAP_H

#include <map>
#include "utils/CicadaType.h"

using namespace std;

class CICADA_CPLUS_EXTERN ErrorCodeMap {

public:

    static ErrorCodeMap* sInstance;
    static ErrorCodeMap* getInstance();

    int getValue(int orignalValue);

    bool containsCode(int orignalValue);

private:
    ErrorCodeMap();

    ~ErrorCodeMap();

    void init();

    map<int, int>       codeMap;
};


#endif //SOURCE_ERRORCODEMAP_H
