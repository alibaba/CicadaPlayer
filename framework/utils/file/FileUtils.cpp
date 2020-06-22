//
// Created by lifujun on 2019/2/15.
//

#include <memory.h>
#ifdef _WIN32
    #include <io.h>
    #include <process.h>
    #include <direct.h>
#else
    #include <unistd.h>
#endif
#include <sys/stat.h>
#include <dirent.h>
#include <cstdio>
#include "FileUtils.h"
#include <cstdlib>
#include <ctime>
#include <stdint.h>

extern "C" {
#include <cerrno>
}

namespace Cicada {

    static FileUtils a;

    int FileUtils::isDirExist(const char *dir_path)
    {
        if (dir_path == nullptr) {
            return -1;
        }

        DIR *dir = opendir(dir_path);

        if (dir == nullptr) {
            return -1;
        }

        closedir(dir);
        return FILE_TRUE;
    }

    int FileUtils::isFileExist(const char *file_path)
    {
        if (file_path == nullptr) {
            return -1;
        }

#ifdef _WIN32

        if (access(file_path, _A_NORMAL) != -1) {
#else

        if (access(file_path, F_OK) != -1) {
#endif
            return FILE_TRUE;
        }

        return -1;
    }

    long FileUtils::getFileLength(const char *filePath)
    {
        struct stat fileStat {};
        int64_t       ret = stat(filePath, &fileStat);

        if (ret == 0) {
            return fileStat.st_size;
        }

        return ret;
    }

    int FileUtils::touch(const char *fileAbsPath)
    {
        FILE *file = fopen(fileAbsPath, "ab+");

        if (file == nullptr) {
            return -1;
        }

        fclose(file);
        return FILE_TRUE;
    }

    char *FileUtils::path_normalize(const char *path)
    {
        if (!path) { return nullptr; }

        char *copy = strdup(path);

        if (nullptr == copy) { return nullptr; }

        char *ptr = copy;
        int  i    = 0;

        for (i = 0; copy[i]; i++) {
            *ptr++ = path[i];

            if (PATH_SEPARATION == path[i]) {
                i++;

                while (PATH_SEPARATION == path[i]) { i++; }

                i--;
            }
        }

        *ptr = '\0';
        return copy;
    }

    //copy from https://github.com/stephenmathieson/mkdirp.c
    int FileUtils::mkdirs(const char *path)
    {
        //1.if already exited.
        if (isDirExist(path) == 0) {
            return FILE_TRUE;
        }

        //2.not created yet.
        char *pathname = nullptr;
        char *parent   = nullptr;

        if (nullptr == path) { return -1; }

        pathname = path_normalize(path);

        if (nullptr == pathname) {
            free(pathname);
            free(parent);
            return -1;
        }

        parent = strdup(pathname);

        if (nullptr == parent) {
            free(pathname);
            free(parent);
            return -1;
        }

        char *p = parent + strlen(parent);

        while (PATH_SEPARATION != *p && p != parent) {
            p--;
        }

        *p = '\0';

        // make parent dir
        if (p != parent && 0 != mkdirs(parent)) {
            free(pathname);
            free(parent);
            return -1;
        }

        free(parent);
        // make this one if parent has been made
#ifdef _WIN32
        // http://msdn.microsoft.com/en-us/library/2fkk4dzw.aspx
        int rc = _mkdir(pathname);
#else
        //S_IRWXU|S_IRWXG|S_IRWXO = 511
        int rc = mkdir(pathname, 511);
#endif
        free(pathname);
        return 0 == rc || EEXIST == errno
               ? FILE_TRUE
               : -1;
    }

    int FileUtils::rmrf(const char *targetPath)
    {
        if (isDirExist(targetPath) == 0) {
            DIR           *dir;
            struct dirent *entry;
            char          path[UTILS_PATH_MAX];
            dir = opendir(targetPath);

            if (dir == nullptr) {
                return -1;
            }

            while ((entry = readdir(dir)) != nullptr) {
                if (strcmp((const char *) (entry->d_name), ".") &&
                        strcmp((const char *) (entry->d_name), (const char *) "..")) {
                    snprintf(path, (size_t) UTILS_PATH_MAX, "%s%c%s", targetPath, PATH_SEPARATION,
                             entry->d_name);
                    rmrf(path);
                }
            }

            closedir(dir);
            rmdir(targetPath);
        } else if (isFileExist(targetPath) == FILE_TRUE) {
            unlink(targetPath);
        }

        return FILE_TRUE;
    }

    long FileUtils::getFileCreateTime(const char *filePath)
    {
        struct stat buf {};
        int         result;
        result = stat(filePath, &buf);

        if (result < 0) {
            return -1;
        }

        return buf.st_ctime;
    }

    int FileUtils::Rename(const char *oldName, const char *newName)
    {
        int ret = rename(oldName, newName);//success == 0;
        return ret;
    }
}
