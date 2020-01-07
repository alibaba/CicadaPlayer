//
// Created by moqi on 2019/11/1.
//

#include "subTitleSource.h"
#include <data_source/dataSourcePrototype.h>
#include <utils/errors/framework_error.h>
#include <cerrno>

#include <utility>

using namespace std;

namespace Cicada {

    subTitleSource::subTitleSource(string uri) : mUrl(std::move(uri))
    {
    }

    subTitleSource::~subTitleSource()
    {
    }

    int subTitleSource::open()
    {
        mDataSource = unique_ptr<IDataSource>(dataSourcePrototype::create(mUrl, mOpts));

        if (mDataSource == nullptr) {
            return FRAMEWORK_ERR_PROTOCOL_NOT_SUPPORT;
        }

        IDataSource::SourceConfig config{};
        config.low_speed_time_ms = 15000;
        config.low_speed_limit = 1;
        config.connect_time_out_ms = 15000;
        mDataSource->Set_config(config);
        int ret = mDataSource->Open(0);

        if (ret < 0) {
            return ret;
        }

        mDemuxer = unique_ptr<demuxer_service>(new demuxer_service(mDataSource.get()));
        mDemuxer->setOptions(mOpts);
        ret = mDemuxer->initOpen();

        if (ret < 0) {
            return ret;
        }

        int nbStream = mDemuxer->GetNbStreams();

        if (nbStream <= 0) {
            return -EINVAL;
        }

        unique_ptr<streamMeta> smeta;
        int i;

        for (i = 0; i < nbStream; ++i) {
            mDemuxer->GetStreamMeta(smeta, i, false);
            auto *meta = (Stream_meta *) (*(smeta.get()));

            if (meta->type == STREAM_TYPE_SUB) {
                mDemuxer->OpenStream(i);
                break;
            }
        }

        if (i == nbStream) {
            return -EINVAL;
        }

        return 0;
    }

    int subTitleSource::read(std::unique_ptr<IAFPacket> &packet)
    {
        if (mDemuxer == nullptr) {
            return -EINVAL;
        }

        int ret = mDemuxer->readPacket(packet, 0);

        if (packet) {
            packet->getInfo().streamIndex = mId;
        }

        return ret;
    }

    int subTitleSource::seek(int64_t pts)
    {
        if (mDemuxer == nullptr) {
            return -EINVAL;
        }

        return mDemuxer->Seek(pts, 0, -1);
    }

    void subTitleSource::setID(int id)
    {
        mId = id;
    }

    int subTitleSource::getID()
    {
        return mId;
    }

}
