//
// Created by moqi on 2019/11/22.
//

#ifndef CICADAPLAYERSDK_OPTIONOWNER_H
#define CICADAPLAYERSDK_OPTIONOWNER_H

#include "options.h"

namespace Cicada {
    class OptionOwner {
    public:
        void setOptions(const options *opts)
        {
            mOpts = opts;
        }

        const options *getOptions()
        {
            return mOpts;
        }

        ~OptionOwner() = default;

    protected:
        const options *mOpts = nullptr;

    };
}


#endif //CICADAPLAYERSDK_OPTIONOWNER_H
