//
// Created by moqi on 2019-04-18.
//

#include "proxyDataSource.h"

using std::string;

namespace Cicada {

    proxyDataSource::proxyDataSource() : IDataSource("")
    {
    }

    proxyDataSource::~proxyDataSource()
    {
    }

    int proxyDataSource::Open(int flags)
    {
        return -1;
    }

    int proxyDataSource::Open(const string &url)
    {
        if (mOpen) {
            return mOpen(mUserArg, url.c_str(), rangeStart, rangeEnd);
        }

        return -1;
    }

    void proxyDataSource::Close()
    {
    }

    int64_t proxyDataSource::Seek(int64_t offset, int whence)
    {
        if (mSeek) {
            return mSeek(mUserArg, offset, whence);
        }

        return -1;
    }

    int proxyDataSource::Read(void *buf, size_t nbyte)
    {
        if (mRead) {
            return mRead(mUserArg, static_cast<uint8_t *>(buf), nbyte);
        }

        return -1;
    }

    void proxyDataSource::setImpl(readImpl read, seekImpl seek, openImpl open, interruptImpl interrupt, setSegmentListImpl setSegmentList,
                                  getBufferDurationImpl getBufferDuration, enableCacheImpl enableCache, void *arg)
    {
        mOpen = open;
        mRead = read;
        mSeek = seek;
        mUserArg = arg;
        mInter = interrupt;
        mSetSegmentList = setSegmentList;
        mGetBufferDuration = getBufferDuration;
        mEnableCache = enableCache;
    }

    void proxyDataSource::Interrupt(bool interrupt)
    {
        if (mInter) {
            return mInter(mUserArg, interrupt);
        }
    }

    void proxyDataSource::setSegmentList(const std::vector<mediaSegmentListEntry> &segments)
    {
        if (mSetSegmentList) {
            mSetSegmentList(mUserArg, segments);
        }
    }

    void proxyDataSource::enableCache(const std::string &originUrl, bool enable)
    {
        if (mEnableCache) {
            mEnableCache(mUserArg, originUrl, enable);
        }
    }

    int64_t proxyDataSource::getBufferDuration()
    {
        if (mGetBufferDuration) {
            return mGetBufferDuration(mUserArg, 0);
        }
        return 0;
    }
}
