//
// Created by lifujun on 2019/4/3.
//

#ifndef SOURCE_EVENTCODEMAP_H
#define SOURCE_EVENTCODEMAP_H

#include <map>
#include "utils/CicadaType.h"

using namespace std;

class CICADA_CPLUS_EXTERN EventCodeMap {

public:

    static EventCodeMap* sEventCodeInstance;
    static EventCodeMap* getInstance();

    int getValue(int orignalValue);

private:
    EventCodeMap();

    ~EventCodeMap();

    void init();

    map<int, int>       codeMap;
};

#endif //SOURCE_EVENTCODEMAP_H
