//
// Created by yuyuan on 2021/03/08.
//

#ifndef DEMUXER_DASH_MPDPARSER_H
#define DEMUXER_DASH_MPDPARSER_H

#include "MPDPlayList.h"
#include "demuxer/play_list/playList.h"
#include "demuxer/play_list/playListParser.h"
#include "utils/xml/Node.h"

namespace Cicada {

    class Period;
    class AdaptationSet;

    namespace Dash {
        class SegmentInformation;
        class ISegmentBase;

        class MPDParser : public playListParser {
        public:
            MPDParser(std::string url);
            ~MPDParser();

            virtual playList *parse(const std::string &playlistur) override;

            static int probe(const uint8_t *buffer, int size);

        private:
            static void parseMPDAttributes(MPDPlayList *mpd, xml::Node *node);
            void parseProgramInformation(MPDPlayList *, xml::Node *node);
            void parseMPDBaseUrl(MPDPlayList *mpd, xml::Node *root);
            void parsePeriods(MPDPlayList *mpd, xml::Node *root);
            size_t parseSegmentInformation(MPDPlayList *mpd, xml::Node *node, SegmentInformation *info, uint64_t *nextid);
            size_t parseSegmentBase(MPDPlayList *mpd, xml::Node *segmentBaseNode, SegmentInformation *info);
            void parseAdaptationSets(MPDPlayList *mpd, xml::Node *periodNode, Period *period);
            void parseRepresentations(MPDPlayList *mpd, xml::Node *adaptationSetNode, AdaptationSet *adaptationSet);
            size_t parseSegmentList(MPDPlayList *mpd, xml::Node *segListNode, SegmentInformation *info);
            size_t parseSegmentTemplate(MPDPlayList *mpd, xml::Node *templateNode, SegmentInformation *info);
            void parseTimeline(MPDPlayList *mpd, xml::Node *node, ISegmentBase *base);
            void parseCommonSegmentBase(MPDPlayList *mpd, xml::Node *node, ISegmentBase *base, SegmentInformation *parent);
            void parseCommonMultiSegmentBase(MPDPlayList *mpd, xml::Node *node, ISegmentBase *base, SegmentInformation *parent);
            void parseInitSegment(xml::Node *initNode, ISegmentBase *base, SegmentInformation *parent);
            void parseBaseUrl(MPDPlayList *mpd, xml::Node *containerNode, SegmentInformation *parent);

        private:
            xml::Node *mRoot = nullptr;
            std::string playlisturl;
        };
    }// namespace Dash
}// namespace Cicada


#endif//CICADAMEDIA_MPDPARSER_H
