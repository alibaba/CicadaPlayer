//
// Created by lifujun on 2019/2/15.
//

#ifndef CICADA_FILEUTILS_H
#define CICADA_FILEUTILS_H

#include "utils/CicadaType.h"

#ifdef _WIN32
#define PATH_SEPARATION   '\\'
#else
#define PATH_SEPARATION   '/'
#endif

#define FILE_TRUE 0
#define UTILS_PATH_MAX 4096

namespace Cicada {

    class CICADA_CPLUS_EXTERN FileUtils {
        public:
        static int isFileExist(const char *file_path);

        static int isDirExist(const char *dirAbsPath);

        static long getFileLength(const char *filePath);

        static int touch(const char *fileAbsPath);

        static int mkdirs(const char *dirAbsPath);

        static int rmrf(const char *targetPath);

        static char * path_normalize(const char *path);

        static long getFileCreateTime(const char* filePath);

        static int Rename(const char* oldName, const char* newName);
    };
}

#endif //CICADA_FILEUTILS_H
