//
// Created by moqi on 2018/4/25.
//

#ifndef FRAMEWORK_HLSPARSER_H
#define FRAMEWORK_HLSPARSER_H

#include "playListParser.h"
#include "../../data_source/dataSourceIO.h"
#include "HlsTags.h"
#include <string>

using namespace std;
using namespace Cicada::hls;
namespace Cicada{
    class HlsParser : public playListParser {
    public:
        explicit HlsParser(const char *uri);

        ~HlsParser() override;

        static int probe(const uint8_t *buffer, int size);

        playList *parse(const std::string &playlistur) override;

        void createAndFillRepresentation(dataSourceIO *stream, AdaptationSet *adaptSet,
                                         const AttributesTag *tag,
                                         const std::list<Tag *> &tagslist);

        void parseSegments(dataSourceIO *stream, Representation *rep, const std::list<Tag *> &tagslist);

    private:

        Representation *createRepresentation(AdaptationSet *adaptSet, const AttributesTag *tag);

        std::list<Tag *> parseEntries(dataSourceIO *stream);

        char *mBuffer = nullptr;


    };
}


#endif //FRAMEWORK_HLSPARSER_H
