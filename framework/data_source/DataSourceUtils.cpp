//
//  DataSourceUtils.cpp
//  data source
//
//  Created by huang_jiafa on 2019/1/03.
//  Copyright (c) 2019 Aliyun. All rights reserved.
//

#include "DataSourceUtils.h"
#include <sstream>
#include "utils/timer.h"

namespace Cicada {

    std::string DataSourceUtils::getPropertryOfResponse(const std::string &response, const std::string &key)
    {
        std::size_t pos = response.find(key);

        if (pos != std::string::npos) {
            std::string subString = response.substr(pos + key.length());
            pos = subString.find("\r\n");

            if (pos != std::string::npos) {
                std::size_t from = subString.find_first_not_of(" ");

                if (std::string::npos == from) {
                    from = 0;
                }

                return subString.substr(from, pos - from);
            }
        }

        return "";
    }

}// namespace Cicada
