//
// Created by SuperMan on 12/2/20.
//

#include "DrmUtils.h"

bool DrmUtils::isSupport(const std::string &drmFormat) {
#ifdef ANDROID
    return drmFormat == "urn:uuid:edef8ba9-79d6-4ace-a3c8-27dcd51d21ed";
#endif
    return false;
}
