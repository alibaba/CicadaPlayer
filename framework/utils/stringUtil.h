//
// Created by lifujun on 2018/11/21.
//

#ifndef SAASLIBRARY_STRINGUTIL_H
#define SAASLIBRARY_STRINGUTIL_H

#include <sstream>
#include <string>
#include <cstdlib>

using namespace std;

class stringUtil {
public:
    template<typename T>
    static std::string to_string(T value) {
#ifdef ANDROID
        std::ostringstream os;
        os << value;
        return os.str();
#else
        return std::to_string(value);
#endif
    }

    static int to_int(string str){
        int value = 0;
        std::istringstream strm(str);
        strm >> value;
        return value;
    }

    static double to_double(string str) {
        double value = 0;
        std::istringstream strm(str);
        strm >> value;
        return value;
    }

};

#endif //SAASLIBRARY_STRINGUTIL_H
