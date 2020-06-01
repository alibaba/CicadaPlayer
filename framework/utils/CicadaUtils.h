//
//  CicadaUtils.hpp
//  framework_utils
//
//  Created by jeff on 2019/4/10.
//

#ifndef CicadaUtils_hpp
#define CicadaUtils_hpp

#include <stdio.h>
#include <string>
#include "CicadaType.h"
#include <vector>

class CICADA_CPLUS_EXTERN CicadaUtils {
public:
    static bool isEqual(double a, double b);

    static bool startWith(const std::string &src, const std::initializer_list<std::string > &val);

    static std::vector<std::string> split(const std::string &str, char delim);

    static std::string base64enc(const std::string &str);

    static std::string base64dec(const std::string &str);

    static std::string base64enc(const char* in, int len);

    static int base64dec(const std::string &str , char** dst);

};

#endif /* CicadaUtils_hpp */
