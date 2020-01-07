//
// Created by moqi on 2019/11/5.
//
#define LOG_TAG "BiDataSource"

#include "BiDataSource.h"
#include <cerrno>
#include <string>
#include <utils/frame_work_log.h>
#include <utils/errors/framework_error.h>
#include <cstdlib>
#include <cassert>

#define header  "bitp://"

using namespace std;

//bittp://base64_1:base64_2:size

using namespace Cicada;


BiDataSource BiDataSource::se(0);


std::string BiDataSource::createUrl(const std::string &first, const std::string &second, uint64_t size)
{
    return header + CicadaUtils::base64enc(first) + ":" + CicadaUtils::base64enc(second) + ":" + to_string(size);
}

BiDataSource::BiDataSource(const std::string &url) : IDataSource(url)
{
}

BiDataSource::~BiDataSource()
{

}

int BiDataSource::Open(int flags)
{
    vector<string> strs = CicadaUtils::split(mUri.substr(strlen(header)), ':');

    if (strs.size() < 2) {
        return -EINVAL;
    }

    if (strs.size() >= 3) {
        fileSize = atoll(strs[2].c_str());
    }
    for (int i = 0; i < 2; ++i) {
        unique_ptr<source> pSource = unique_ptr<source>(new source());
        pSource->mUri = CicadaUtils::base64dec(strs[i]);
        pSource->mDataSource = unique_ptr<IDataSource>(dataSourcePrototype::create(pSource->mUri,mOpts));
        if (pSource->mDataSource->getSpeedLevel() == speedLevel_local) {
            int ret = pSource->mDataSource->Open(0);
            if (ret >= 0) {
                int64_t size = pSource->mDataSource->Seek(0, SEEK_SIZE);
                if (size > 0) {
                    pSource->mRange.end = static_cast<uint64_t>(size);
                }
                pSource->mIsOpened = true;
                addSource(pSource);
            }
        } else {
            addSource(pSource);
        }
    }

    assert(!mSources.empty());
    if (mSources.empty())
        return -EINVAL;
    if (mSources.size() == 1) {
        assert(mSources[0]->mDataSource->getSpeedLevel() != speedLevel_local);
        if (mSources[0]->mDataSource->getSpeedLevel() == speedLevel_local) {
            return -EINVAL;
        }

        if (!mSources[0]->mIsOpened) {
            int ret = mSources[0]->mDataSource->Open(0);
            if (ret < 0) {
                AF_LOGE("open source error %0x\n", ret);
                std::unique_lock<std::mutex> uMutex(mSourceMutex);
                mSources.erase(mSources.begin());
                return ret;
            }
            mSources[0]->mIsOpened = true;
        }

        mCurrent = mSources[0].get();

        fileSize = mCurrent->mDataSource->Seek(0, SEEK_SIZE);
    } else {
        if (mSources[0]->mDataSource->getSpeedLevel() == speedLevel_local)
            mCurrent = mSources[0].get();
        else
            mCurrent = mSources[1].get();
    }
    return 0;
}

void BiDataSource::addSource(unique_ptr<source> &pSource)
{
    pSource->mDataSource->Set_config(mConfig);
    pSource->mDataSource->Interrupt(mInterrupt);
    unique_lock<mutex> uMutex(mSourceMutex);
    mSources.push_back(move(pSource));
}

int BiDataSource::Open(const std::string &url)
{
    return -ENOTSUP;
}

void BiDataSource::Close()
{
    for (auto &item: mSources) {
        if (item->mIsOpened)
            item->mDataSource->Close();
    }
}

int64_t BiDataSource::getFileSize()
{

    int64_t size = -1;

    for (auto item = mSources.begin(); item != mSources.end(); ++item) {
        if ((*item)->mDataSource->getSpeedLevel() == speedLevel_remote) {
            if (!(*item)->mIsOpened) {
                int ret = (*item)->mDataSource->Open(0);
                if (ret < 0) {
                    std::unique_lock<std::mutex> uMutex(mSourceMutex);
                    mSources.erase(item);
                } else {
                    (*item)->mIsOpened = true;
                    size = (*item)->mDataSource->Seek(0, SEEK_SIZE);
                }
            }
            break;
        }
    }
    return size;
}

int64_t BiDataSource::Seek(int64_t offset, int whence)
{
    int64_t ret;

    if (mCurrent == nullptr)
        return -EINVAL;

    if (whence == SEEK_SIZE) {
        if (fileSize == 0) {
            fileSize = getFileSize();
        }
        return fileSize;
    } else if ((whence == SEEK_CUR && offset == 0) ||
               (whence == SEEK_SET && offset == fileSize)) {
        if (fileSize == 0) {
            fileSize = getFileSize();
        }
        return fileSize;
    } else if ((fileSize <= 0 && whence == SEEK_END)) {
        return FRAMEWORK_ERR(ENOSYS);
    }

    if (whence == SEEK_CUR) {
        offset += filePos;
    } else if (whence == SEEK_END) {
        if (fileSize == 0) {
            fileSize = getFileSize();
        }
        if (fileSize < 0)
            return -EINVAL;
        offset += fileSize;
    } else if (whence != SEEK_SET) {
        return FRAMEWORK_ERR(EINVAL);
    }

    if (offset < 0) {
        return -(EINVAL);
    }

    if (offset == filePos) {
        return offset;
    }

    if (mSources.size() > 1) {
        mCurrent = nullptr;
    }

    if (mCurrent == nullptr) { // first try local
        source *local = mSources[0]->mDataSource->getSpeedLevel() == speedLevel_local ? mSources[0].get() : mSources[1].get();
        if (local->mRange.end > offset) {
            mCurrent = local;
        }
    }
    if (mCurrent == nullptr) { //change current to remote
        mCurrent = mSources[0]->mDataSource->getSpeedLevel() == speedLevel_remote ? mSources[0].get() : mSources[1].get();
    }

    if (!mCurrent->mIsOpened) {
        ret = mCurrent->mDataSource->Open(0);
        if (ret < 0)
            return ret;
        mCurrent->mIsOpened = true;
    }
    ret = mCurrent->mDataSource->Seek(offset, SEEK_SET);
    if (ret >= 0) {
        filePos = offset;
    }
    return ret;
}

int BiDataSource::Read(void *buf, size_t nbyte)
{
    if (mCurrent == nullptr)
        return -EINVAL;
    int ret = mCurrent->mDataSource->Read(buf, nbyte);
    if (ret > 0) {
        filePos += ret;
        return ret;
    }

    if (mCurrent->mDataSource->getSpeedLevel() != speedLevel_local || ret < 0) {
        return ret;
    }

    // ret == 0

    // local file
    if (fileSize == 0) {
        fileSize = getFileSize();
        if (fileSize <= 0)
            return 0;
    }

    if (filePos >= mCurrent->mRange.end) {
        // change to remote
        mCurrent = mSources[0].get() == mCurrent ? mSources[1].get() : mSources[0].get();
        if (!mCurrent->mIsOpened) {
            ret = mCurrent->mDataSource->Open(0);
            mCurrent->mIsOpened = true;
            if (ret < 0)
                return ret;
        }
        ret = static_cast<int>(mCurrent->mDataSource->Seek(filePos, SEEK_SET));
        if (ret < 0)
            return ret;
        ret = mCurrent->mDataSource->Read(buf, nbyte);
    }
    if (ret > 0)
        filePos += ret;
    return ret;
}

std::string BiDataSource::GetOption(const std::string &key)
{
    if (mCurrent)
        return mCurrent->mDataSource->GetOption(key);
    return IDataSource::GetOption(key);
}

bool BiDataSource::probe(const std::string &uri)
{
    return CicadaUtils::startWith(uri, {header});
}

void BiDataSource::Interrupt(bool interrupt)
{
    std::unique_lock<std::mutex> uMutex(mSourceMutex);
    for (auto &item :mSources) {
        if ((*item).mDataSource) {
            (*item).mDataSource->Interrupt(interrupt);
        }
    }
    IDataSource::Interrupt(interrupt);
}
