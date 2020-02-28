//
// Created by moqi on 2019/11/1.
//

#ifndef CICADAPLAYERSDK_SUBTITLESOURCE_H
#define CICADAPLAYERSDK_SUBTITLESOURCE_H

#include <data_source/IDataSource.h>
#include <demuxer/demuxer_service.h>
#include <base/OptionOwner.h>

namespace Cicada {

    class subTitleSource : public OptionOwner {
    public:
        explicit subTitleSource(std::string uri);

        ~subTitleSource();

        int open();

        int read(std::unique_ptr<IAFPacket> &packet);

        int seek(int64_t pts);

        void setID(int id);

        void close()
        {
            if (mDataSource)
                mDataSource->Interrupt(true);
            if (mDemuxer)
                mDemuxer->interrupt(true);
        }

        int getID();

        std::string getUri()
        {
            return mUrl;
        }

//    private:
//        class dataSourceListener : public Cicada::IDataSource::Listener {
//            NetWorkRetryStatus onNetWorkRetry(int error) override
//            {
//
//            }
//        };

    private:
        std::unique_ptr<IDataSource> mDataSource{nullptr};
        std::unique_ptr<demuxer_service> mDemuxer{nullptr};
        int mId = -1;
        std::string mUrl;

    };
}


#endif //CICADAPLAYERSDK_SUBTITLESOURCE_H
