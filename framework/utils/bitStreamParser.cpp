#include <cstdlib>
#include <cstring>

extern "C" {
#include <libavcodec/avcodec.h>
}

#include  <utils/ffmpeg_utils.h>
#include "bitStreamParser.h"
#include "AFMediaType.h"

namespace Cicada {

    typedef struct parserContent_t {

        AVCodecParserContext *parser;
        AVCodec *codec;
        AVCodecContext *c;
        AVPacket out_pkt;
        enum AFCodecID mId;

    } parserContent;

    bitStreamParser::bitStreamParser()
    {
        mCont = static_cast<parserContent *>(malloc(sizeof(parserContent)));
        memset(mCont, 0, sizeof(parserContent));
    }

    bitStreamParser::~bitStreamParser()
    {
        avcodec_free_context(&mCont->c);
        av_parser_close(mCont->parser);
        av_packet_unref(&mCont->out_pkt);
        free(mCont);
    }

    int bitStreamParser::init(const Stream_meta *meta)
    {
        mCont->mId = meta->codec;
        mCont->parser = av_parser_init(CodecID2AVCodecID(mCont->mId));

        if (!mCont->parser) {
            return -1;
        }

        mCont->parser->flags |= PARSER_FLAG_COMPLETE_FRAMES;
        mCont->codec = avcodec_find_decoder((enum AVCodecID) CodecID2AVCodecID(mCont->mId));
        mCont->c = avcodec_alloc_context3(mCont->codec);

        if (meta->extradata) {
            mCont->c->extradata = static_cast<uint8_t *>(av_malloc(meta->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE));
            mCont->c->extradata_size = meta->extradata_size;
            memcpy(mCont->c->extradata, meta->extradata, meta->extradata_size);
        }

        av_init_packet(&mCont->out_pkt);
        return 0;
    }

    int bitStreamParser::parser(uint8_t *data, int size)
    {
        int ret = av_parser_parse2(mCont->parser, mCont->c, &mCont->out_pkt.data, &mCont->out_pkt.size,
                                   data, size,
                                   AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        return ret;
    }

    InterlacedType bitStreamParser::getInterlaced()
    {
        if (mCont->mId != AF_CODEC_ID_H264) {
            return InterlacedType_NO;
        }

        if (mCont->parser->field_order == AV_FIELD_PROGRESSIVE ||
                mCont->parser->picture_structure == AV_PICTURE_STRUCTURE_FRAME) {
            return InterlacedType_NO;
        } else if (mCont->parser->field_order != AV_FIELD_UNKNOWN
                   || mCont->parser->picture_structure != AV_PICTURE_STRUCTURE_UNKNOWN) {
            return InterlacedType_YES;
        }

        return InterlacedType_UNKNOWN;
    }

    int bitStreamParser::getPOC()
    {
        int poc =  mCont->parser->output_picture_number;
        mCont->parser->output_picture_number = -1;
        return poc;
    }

    void bitStreamParser::getPictureSize(int &width, int &height)
    {
        width =  mCont->parser->width;
        height =  mCont->parser->height;
    }
}
