//
// Created by lifujun on 2019/3/1.
//

#ifndef SOURCE_METATOCODEC_H
#define SOURCE_METATOCODEC_H

extern "C" {
#include <libavformat/avformat.h>
};

#include <utils/AFMediaType.h>

class MetaToCodec {
public:

    static void videoMetaToStream(AVStream *st, Stream_meta *meta);

    static void audioMetaToStream(AVStream *st, Stream_meta *meta);
};


#endif //SOURCE_METATOCODEC_H
