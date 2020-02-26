//
// Created by moqi on 2018/2/1.
//

#ifndef FRAMEWORK_DEMUXER_SERVICE_H
#define FRAMEWORK_DEMUXER_SERVICE_H
#define    MIN(a, b) (((a)<(b))?(a):(b))

#include "base/media/framework_type.h"
#include <data_source/IDataSource.h>
#include <functional>
#include <base/media/IAFPacket.h>
#include "utils/mediaFrame.h"
#include "utils/mediaTypeInternal.h"
#include "DemuxerMeta.h"
#include "IDemuxer.h"
#include <base/OptionOwner.h>
#include <memory>
#include <utils/CicadaType.h>
#include "sample_decrypt/ISampleDecryptor.h"
#define GEN_STREAM_ID(index, subId) (((subId) << 16) + (index))
#define GEN_SUB_STREAM_ID(index) (index >> 16)
#define GEN_STREAM_INDEX(index) (index & 0xFF)

namespace Cicada {

    class CICADA_CPLUS_EXTERN demuxer_service : public OptionOwner {

    public:
        explicit demuxer_service(IDataSource *pDataSource);

        void setNoFile(bool noFile)
        {
            mNoFile = noFile;
        }

        //     demuxer_service(string path);

        ~demuxer_service();

        void SetDataCallBack(demuxer_callback_read read, void *read_arg, demuxer_callback_seek seek,
                             void *seek_arg,
                             const char *uri);

        void setSampleDecryptor(ISampleDecryptor *decryptor)
        {
            mSDec = decryptor;
        };

        int createDemuxer(demuxer_type type);

        int initOpen(demuxer_type type = demuxer_type_unknown);


        int readPacket(std::unique_ptr<IAFPacket> &packet, int index = -1);

        void close();

        void flush();

        int start();

        void stop();

        void preStop();

        int Seek(int64_t us, int flags, int index);

        static int read_callback(void *arg, uint8_t *buffer, int size);

        static int64_t seek_callback(void *arg, int64_t offset, int whence);

        static int open_callback(void *arg, const char *url, int64_t start, int64_t end);

        static void interrupt_callback(void *arg, int inter);

        std::string GetProperty(int index, const std::string &key);

        int SetOption(const std::string &key, int64_t value);

        bool isPlayList();

        void setDemuxerCb(const std::function<void(std::string, std::string)> &func);

        void setDemuxerMeta(std::unique_ptr<DemuxerMeta> &meta);
    public:


        int GetNbStreams();

        int GetSourceMeta(Source_meta **meta);

        attribute_deprecated
        int GetStreamMeta(Stream_meta *meta, int index, bool sub);

        int GetStreamMeta(std::unique_ptr<streamMeta> &meta, int index, bool sub);

        int OpenStream(int index);

        void CloseStream(int index);

        int GetNbSubStream(int index);

        int GetRemainSegmentCount(int index);

        int SwitchStreamAligned(int from, int to);

        void interrupt(int inter);

        void *getCodecPar(int streamIndex);

        IDemuxer *getDemuxerHandle();

    private:
        std::unique_ptr <IDemuxer> mDemuxerPtr {nullptr};
        IDataSource *mPDataSource = nullptr;

        demuxer_callback_read mReadCb = nullptr;
        void *mReadArg{nullptr};

        demuxer_callback_seek mSeekCb = nullptr;
        void *mSeekArg{nullptr};

        std::function<void(std::string, std::string)> mDemuxerCbfunc;

        uint8_t *mPProbBuffer = nullptr;
        int mProbBufferSize = 0;
        uint64_t curPos = 0;
        int64_t mFirstSeekUs = 0;
        bool mNoFile = false;

        ISampleDecryptor *mSDec = nullptr;

        std::unique_ptr<Cicada::DemuxerMeta> mDemuxerMeta;
    };
}

#endif //FRAMEWORK_DEMUXER_SERVICE_H
