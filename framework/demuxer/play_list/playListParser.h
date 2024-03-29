//
// Created by moqi on 2018/4/25.
//

#ifndef FRAMEWORK_PLAYLISTPARSER_H
#define FRAMEWORK_PLAYLISTPARSER_H

#include "base/media/framework_type.h"
#include "playList.h"
#include <cstdint>
#include <data_source/dataSourceIO.h>
#include <string>

using namespace std;
namespace Cicada{
    class playList;

    class playListParser {
    public:
        playListParser() = default;

        virtual ~playListParser();

        virtual playList *parse(const std::string &playlistur) = 0;


        // TODO: use one api only

        void SetDataCallBack(demuxer_callback_read read, demuxer_callback_seek seek, void *seek_arg);

        void setDataSourceIO(dataSourceIO *pData)
        {
            mDataSourceIO = pData;
            mUseCallBack = false;
        };

    protected:
        string mUrl = "";
        demuxer_callback_read mReadCb;
        void *mCBArg{nullptr};

        demuxer_callback_seek mSeekCb;
        dataSourceIO *mDataSourceIO = nullptr;
        bool mUseCallBack = false;

    };
}


#endif //FRAMEWORK_PLAYLISTPARSER_H
