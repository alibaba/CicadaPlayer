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
#include "FileUtils.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <stdint.h>
#include <string>
#include <sys/stat.h>

extern "C" {
#include <cerrno>
}
using namespace std;

namespace Cicada {

    static FileUtils a;

    bool FileUtils::isDirExist(const char *dir_path)
    {
        if (dir_path == nullptr) {
            return false;
        }

        DIR *dir = opendir(dir_path);

        if (dir == nullptr) {
            return false;
        }

        closedir(dir);
        return true;
    }

    bool FileUtils::isFileExist(const char *file_path)
    {
        if (file_path == nullptr) {
            return false;
        }

#ifdef _WIN32

        if (access(file_path, _A_NORMAL) != -1) {
#else

        if (access(file_path, F_OK) != -1) {
#endif
            return true;
        }

        return false;
    }

    int64_t FileUtils::getFileLength(const char *filePath)
    {
        struct stat fileStat {};
        int64_t       ret = stat(filePath, &fileStat);

        if (ret == 0) {
            return (int64_t) fileStat.st_size;
        }

        return ret;
    }

    bool FileUtils::touch(const char *fileAbsPath)
    {
        FILE *file = fopen(fileAbsPath, "ab+");

        if (file == nullptr) {
            return false;
        }

        fclose(file);
        return true;
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
    bool FileUtils::mkdirs(const char *path)
    {
        //1.if already exited.
        if (isDirExist(path)) {
            return true;
        }

        //2.not created yet.
        char *pathname = nullptr;
        char *parent   = nullptr;

        if (nullptr == path) {
            return false;
        }

        pathname = path_normalize(path);

        if (nullptr == pathname) {
            free(pathname);
            free(parent);
            return false;
        }

        parent = strdup(pathname);

        if (nullptr == parent) {
            free(pathname);
            free(parent);
            return false;
        }

        char *p = parent + strlen(parent);

        while (PATH_SEPARATION != *p && p != parent) {
            p--;
        }

        *p = '\0';

        // make parent dir
        if (p != parent && !mkdirs(parent)) {
            free(pathname);
            free(parent);
            return false;
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
        return 0 == rc || EEXIST == errno;
    }

    bool FileUtils::rmrf(const char *targetPath)
    {
        if (isDirExist(targetPath)) {
            DIR           *dir;
            struct dirent *entry;
            char          path[UTILS_PATH_MAX];
            dir = opendir(targetPath);

            if (dir == nullptr) {
                return false;
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
        } else if (isFileExist(targetPath)) {
            unlink(targetPath);
        }

        return true;
    }

    int64_t FileUtils::getFileCreateTime(const char *filePath)
    {
        struct stat buf {};
        int         result;
        result = stat(filePath, &buf);

        if (result < 0) {
            return -1;
        }

        return (int64_t) buf.st_ctime;
    }

    bool FileUtils::Rename(const char *oldName, const char *newName)
    {
        int ret = rename(oldName, newName);//success == 0;
        return ret == 0;
    }
    uint64_t FileUtils::getDirSize(const char *path)
    {
        uint64_t size = 0;
        DIR *dir;
        struct dirent *entry;
        dir = opendir(path);
        if (dir == nullptr) {
            return 0;
        }
        string dirPath = path;
        dirPath += PATH_SEPARATION;
        while ((entry = readdir(dir)) != nullptr) {
            string filePath = dirPath + entry->d_name;
            size += getFileLength(filePath.c_str());
        }
        closedir(dir);
        return size;
    }
    void FileUtils::forEachDir(const char *path, const function<void(struct dirent *)> &func)
    {
        DIR *dir;
        struct dirent *entry;
        dir = opendir(path);
        if (dir == nullptr) {
            return;
        }
        while ((entry = readdir(dir)) != nullptr) {
            func(entry);
        }
        closedir(dir);
    }
    static bool pathIsStatus(const char *path, int flag)
    {
        struct stat buf {
        };
        if (lstat(path, &buf) < 0) {
            return false;
        }
        return buf.st_mode & flag;
    }

    bool FileUtils::isDir(const string &path)
    {
        return pathIsStatus(path.c_str(), S_IFDIR);
    }
    bool FileUtils::isRegularFile(const string &path)
    {
        return pathIsStatus(path.c_str(), S_IFREG);
    }
}
