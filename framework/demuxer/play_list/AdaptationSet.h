//
// Created by moqi on 2018/4/25.
//

#ifndef FRAMEWORK_ADAPTATIONSET_H
#define FRAMEWORK_ADAPTATIONSET_H


#include <list>
#include "Representation.h"
#include "Period.h"

//using namespace std;

namespace Cicada{
    class Period;

    class Representation;

    class AdaptationSet {
    public:
        AdaptationSet(Period *period)
        {
            mPeriod = period;
        }

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

    private:
        std::list<Representation *> mRepresentList{};
        std::string mimeType = "";
        Period *mPeriod = nullptr;
        std::string mDescription = "";
    };
}


#endif //FRAMEWORK_ADAPTATIONSET_H
