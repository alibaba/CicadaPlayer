//
// Created by lifujun on 2019/2/15.
//

#ifndef CICADA_FILEUTILS_H
#define CICADA_FILEUTILS_H

#include "utils/CicadaType.h"
#include <cstdint>
#include <string>

#define PATH_SEPARATION '/'

#define UTILS_PATH_MAX 4096
#include <dirent.h>
#include <functional>

namespace Cicada {

    class CICADA_CPLUS_EXTERN FileUtils {
    public:
        static bool isFileExist(const char *file_path);

        static bool isDirExist(const char *dirAbsPath);

        static int64_t getFileLength(const char *filePath);

        static long getFileTime(const char *filePath, int64_t &mtimeSec, int64_t &atimeSec);

        static bool touch(const char *fileAbsPath);

        static bool mkdirs(const char *dirAbsPath);

        static bool rmrf(const char *targetPath);

        static char *path_normalize(const char *path);

        static int64_t getFileCreateTime(const char *filePath);

        static bool Rename(const char *oldName, const char *newName);

        static uint64_t getDirSize(const char *path);

        static void forEachDir(const char *path, const std::function<void(struct dirent *entry)> &);

        static bool getDiskSpaceInfo(const char *path, uint64_t &availableBytes, uint64_t &totalBytes);

        static inline bool isExist(const std::string &path)
        {
            return isFileExist(path.c_str());
        }

        static bool isDir(const std::string &path);

        static bool isRegularFile(const std::string &path);
    };
}// namespace Cicada

#endif//CICADA_FILEUTILS_H
