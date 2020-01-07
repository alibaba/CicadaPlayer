//
// Created by moqi on 2018/4/25.
//

#include "playListParser.h"

namespace Cicada {

    void playListParser::SetDataCallBack(demuxer_callback_read read, demuxer_callback_seek seek,
                                         void *arg)
    {
        mReadCb = read;
        mCBArg = arg;
        mSeekCb = seek;
        mUseCallBack = true;
        mDataSourceIO = nullptr;
    }
}