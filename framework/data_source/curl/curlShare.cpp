//
// Created by moqi on 2019/11/20.
//

#include "curlShare.h"
static int gCurlSharedLive =-1;

Cicada::curlShare::curlShare(uint64_t flags)
{
    mShare = curl_share_init();
    curl_share_setopt(mShare, CURLSHOPT_USERDATA, this);
    curl_share_setopt(mShare, CURLSHOPT_LOCKFUNC, lockData);
    curl_share_setopt(mShare, CURLSHOPT_UNLOCKFUNC, unlockData);
    if (flags & SHARED_SSL_SESSION)
        curl_share_setopt(mShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION);
    if (flags & SHARED_DNS)
        curl_share_setopt(mShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
    gCurlSharedLive = 1;
}

void Cicada::curlShare::lockData(CURL *handle, curl_lock_data data, curl_lock_access access, void *useptr)
{
    auto *pThis = reinterpret_cast<curlShare *>(useptr);
    if (gCurlSharedLive == 1) {
        pThis->mutexes_[data].lock();
    }
}

void Cicada::curlShare::unlockData(CURL *handle, curl_lock_data data, void *useptr)
{
    auto *pThis = reinterpret_cast<curlShare *>(useptr);
    if (gCurlSharedLive == 1) {
        pThis->mutexes_[data].unlock();
    }
}

Cicada::curlShare::~curlShare()
{
    gCurlSharedLive = 0;
    curl_share_cleanup(mShare);
}

Cicada::curlShare::operator CURLSH *() const
{
    return mShare;
}
