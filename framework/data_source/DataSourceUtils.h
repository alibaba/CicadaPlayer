//
//  DataSourceUtils.h
//  data source
//
//  Created by huang_jiafa on 2019/1/03.
//  Copyright (c) 2019 Aliyun. All rights reserved.
//

#ifndef DataSourceUtils_H
#define DataSourceUtils_H

#include <string>

namespace Cicada{

    class DataSourceUtils {
    public:
        static std::string getPropertryOfResponse(const std::string &response, const std::string &key);
    };

}// namespace Cicada

#endif // DataSourceUtils_H

