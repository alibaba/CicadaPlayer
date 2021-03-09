//
// Created by moqi on 2018/4/25.
//

#ifndef FRAMEWORK_ADAPTATIONSET_H
#define FRAMEWORK_ADAPTATIONSET_H

#include "demuxer/dash/SegmentInformation.h"
#include <list>

namespace Cicada{

    class Period;
    class Representation;

    class AdaptationSet : public Dash::SegmentInformation {
    public:
        AdaptationSet(Period *period);
        ~AdaptationSet();

        void addRepresentation(Representation *represent);

        std::list<Representation *> getRepresentations()
        {
            return mRepresentList;
        }

        void setDescription(std::string &desc)
        {
            mDescription = desc;
        }

        std::string getDescription()
        {
            return mDescription;
        }

        Period *getPeriod()
        {
            return mPeriod;
        }

        void print();

        const std::string &getMimeType() const;

    public:
        std::string mimeType;
        std::string lang;
        bool segmentAligned = false;
        bool bitswitchAble = false;

    private:
        std::list<Representation *> mRepresentList{};
        
        Period *mPeriod = nullptr;
        std::string mDescription = "";
    };
}


#endif //FRAMEWORK_ADAPTATIONSET_H
