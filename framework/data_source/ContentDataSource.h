//
// Created by SuperMan on 2020/9/21.
//

#ifndef SOURCE_CONTENTDATASOURCE_H
#define SOURCE_CONTENTDATASOURCE_H

#include <utils/Android/FindClass.h>
#include <utils/Android/JniEnv.h>
#include <utils/Android/NewStringUTF.h>
#include <utils/Android/JniException.h>
#include <utils/af_string.h>
#include <fcntl.h>
#include "IDataSource.h"
#include "dataSourcePrototype.h"

namespace Cicada {
    class ContentDataSource : public IDataSource, private dataSourcePrototype {
    public:

        static bool probe(const std::string &path) {
            return AfString::startWith(path, "content://");
        };

        explicit ContentDataSource(const std::string &url);

        ~ContentDataSource() override;

        int Open(int flags) override;

        void Close() override;

        int64_t Seek(int64_t offset, int whence) override;

        int Read(void *buf, size_t nbyte) override;

        void Interrupt(bool interrupt) override;

        std::string Get_error_info(int error) override;

        std::string GetOption(const std::string &key) override;

        static void init();

        static void unInit();
    private:

    private:

    private:
        explicit ContentDataSource(int dummy) : IDataSource("") {
            addPrototype(this);
        }

        Cicada::IDataSource *clone(const std::string &uri) override {
            return new ContentDataSource(uri);
        };

        bool is_supported(const std::string &uri) override {
            return probe(uri);
        };

        static ContentDataSource se;

    private:

        jobject mJContentDataSource = nullptr;
    };
}


#endif //SOURCE_CONTENTDATASOURCE_H
