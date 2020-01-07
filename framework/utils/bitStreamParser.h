
#include "AFMediaType.h"

#ifndef BIT_STREAM_PARSER_H
#define BIT_STREAM_PARSER_H

namespace Cicada{
    class bitStreamParser {
        typedef struct parserContent_t parserContent;
    public:
        bitStreamParser();

        ~bitStreamParser();

        int init(const Stream_meta *meta);

        int parser(uint8_t *data, int size);

        InterlacedType getInterlaced();

        int getPOC();

        void getPictureSize(int &width, int &height);
    private:
        parserContent *mCont;

    };
}
#endif