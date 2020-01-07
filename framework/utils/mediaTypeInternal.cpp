//
// Created by moqi on 2019-08-20.
//

#include "mediaTypeInternal.h"
#include "mediaFrame.h"

streamMeta::streamMeta(const Stream_meta *meta)
{
    mMeta = *meta;
}

streamMeta::~streamMeta()
{
    releaseMeta(&mMeta);
}

streamMeta::operator Stream_meta *()
{
    return &mMeta;
}
