//
// Created by yiliang on 2021/8/4.
//
#include "IVideoFilter.h"
#include "utils/CicadaType.h"
#include <base/options.h>
#include <base/prototype.h>
#include <utils/CicadaJSON.h>

#ifndef CICADAMEDIA_FILTERPROTOTYPE_H
#define CICADAMEDIA_FILTERPROTOTYPE_H

class CICADA_CPLUS_EXTERN filterPrototype {
private:
    static Cicada::IVideoFilter *filterQueue[10];
    static int _nextSlot;

public:
    static void addPrototype(Cicada::IVideoFilter *se);

    static Cicada::IVideoFilter *create(int feature, CicadaJSONItem &item, const IAFFrame::videoInfo &videoInfo, bool active);

    static const std::string FILTER_CONFIG_KEY_TARGET;
    static const std::string FILTER_CONFIG_KEY_ID;
    static const std::string FILTER_CONFIG_KEY_NAME;
    static const std::string FILTER_CONFIG_KEY_OPTIONS;

public:
    virtual ~filterPrototype() = default;

private:
    static Cicada::IVideoFilter *getMaxScoreFilter(const std::list<Cicada::IVideoFilter *> &filterList);

    static std::list<Cicada::IVideoFilter *> getTargetSupportFilters(const CicadaJSONItem &item, const IAFFrame::videoInfo &videoInfo);

    static void dummy(int dummy);
};

#endif//CICADAMEDIA_FILTERPROTOTYPE_H
