//
// Created by moqi on 2018/2/1.
//

#include "IDemuxer.h"

#include <utility>

namespace Cicada {

    IDemuxer::IDemuxer()
    {
    }

    IDemuxer::~IDemuxer()
    {
    }

    IDemuxer::IDemuxer(string path) : mPath(std::move(path))
    {
    }

    void IDemuxer::SetDataCallBack(demuxer_callback_read read, demuxer_callback_seek seek, demuxer_callback_open open,
                                   demuxer_callback_interrupt_data inter, void *arg)
    {
        mReadCb = read;
        mUserArg = arg;
        mSeekCb = seek;
        mOpenCb = open;
        mInterruptCb = inter;
    }

    int IDemuxer::GetStreamMeta(unique_ptr<streamMeta> &meta, int index, bool sub) const
    {
        Stream_meta Meta;
        int ret = GetStreamMeta(&Meta, index, sub);

        if (ret < 0) {
            return ret;
        }

        meta = unique_ptr<streamMeta>(new streamMeta(&Meta));
        return 0;
    };

}
