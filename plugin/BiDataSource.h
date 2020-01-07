//
// Created by moqi on 2019/11/5.
//

#ifndef CICADAPLAYERSDK_BIDATASOURCE_H
#define CICADAPLAYERSDK_BIDATASOURCE_H

#include <data_source/dataSourcePrototype.h>
#include <utils/CicadaUtils.h>
#include <vector>
#include <mutex>
#include <memory>

class BiDataSource : public Cicada::IDataSource, private dataSourcePrototype {
public:

    explicit BiDataSource(const std::string &url);

    ~BiDataSource();

    static bool probe(const std::string &uri);

    static std::string createUrl(const std::string &first, const std::string &second, uint64_t size);

    int Open(int flags) override;

    int Open(const std::string &url) override;

    void Close() override;

    int64_t Seek(int64_t offset, int whence) override;

    int Read(void *buf, size_t nbyte) override;

    std::string GetOption(const std::string &key) override;

    void Interrupt(bool interrupt) override;

private:

    explicit BiDataSource(int dummy) : IDataSource("")
    {
        addPrototype(this);
    }

    IDataSource *clone(const std::string &uri) override
    {
        return new BiDataSource(uri);
    }

    bool is_supported(const std::string &uri) override
    {
        return probe(uri);
    }

    static BiDataSource se;

private:
    class source {
    public:
        class range {
        public:
            uint64_t start = 0;
            uint64_t end = 0;
        };

        std::string mUri;
        std::unique_ptr<IDataSource> mDataSource;
        IDataSource::speedLevel mLevel = speedLevel_remote;
        range mRange;
        bool mIsOpened{false};
    };

private:
    int64_t getFileSize();

    void addSource(std::unique_ptr<source> &pSource);

private:
    int64_t fileSize{0};
    int64_t filePos = 0;
    std::vector<std::unique_ptr<source>> mSources;
    source *mCurrent{nullptr};
    std::mutex mSourceMutex;
};


#endif //CICADAPLAYERSDK_BIDATASOURCE_H
