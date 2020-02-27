//
// Created by moqi on 2018/4/25.
//
#define LOG_TAG "HlsParser"

#include <cstring>
#include <map>
#include <utils/af_string.h>
#include "HlsParser.h"
#include "Helper.h"
#include "../../utils/frame_work_log.h"
#include "utils/AFMediaType.h"


#define CLOCK_FREQ INT64_C(1000000)

#define  MAX_LINE_SIZE 2048
namespace Cicada {
    using namespace hls;

    HlsParser::HlsParser(const char *uri)
    {
        mUrl = uri;
        mBuffer = new char[MAX_LINE_SIZE];
    }

    HlsParser::~HlsParser()
    {
        delete[] mBuffer;

        if (mUseCallBack) {
            delete mDataSourceIO;
        }
    }

    int HlsParser::probe(const uint8_t *buffer, int size)
    {
        /* Require #EXTM3U at the start, and either one of the ones below
        * somewhere for a proper match. */
        (void) size;

        if (strncmp(reinterpret_cast<const char *>(buffer), "#EXTM3U", 7) != 0) {
            return 0;
        }

        if (strstr(reinterpret_cast<const char *>(buffer), "#EXT-X-STREAM-INF:") ||
                strstr(reinterpret_cast<const char *>(buffer), "#EXT-X-TARGETDURATION:") ||
                strstr(reinterpret_cast<const char *>(buffer), "#EXT-X-MEDIA-SEQUENCE:") ||
                strstr(reinterpret_cast<const char *>(buffer), "#EXT-X-MEDIA:") ||
                strstr(reinterpret_cast<const char *>(buffer), "#EXT-X-STREAM-INF:")
           ) {
            return 100;
        }

        return 0;
    }

    static std::list<Tag *> getTagsFromList(std::list<Tag *> &list, int tag)
    {
        std::list<Tag *> ret;
        std::list<Tag *>::const_iterator it;

        for (it = list.begin(); it != list.end(); ++it) {
            if ((*it)->getType() == tag) {
                ret.push_back(*it);
            }
        }

        return ret;
    }

    Representation *HlsParser::createRepresentation(AdaptationSet *adaptSet, const AttributesTag *tag)
    {
        const Attribute *uriAttr = tag->getAttributeByName("URI");
        const Attribute *bwAttr = tag->getAttributeByName("BANDWIDTH");
        const Attribute *resAttr = tag->getAttributeByName("RESOLUTION");
        const Attribute *codecAttr = tag->getAttributeByName("CODECS");
        const Attribute *videoAttr = tag->getAttributeByName("VIDEO");
        const Attribute *audioAttr = tag->getAttributeByName("AUDIO");
        auto *rep = new Representation(adaptSet);

        if (uriAttr) {
            std::string uri;

            if (tag->getType() == AttributesTag::EXTXMEDIA) {
                uri = uriAttr->quotedString();
            } else {
                uri = uriAttr->value;
            }

            //    rep->setID(uri);
            rep->setPlaylistUrl(uri);

            if (uri.find('/') != std::string::npos) {
                uri = Helper::getDirectoryPath(uri);

                if (!uri.empty()) {
                    //                   AF_LOGD(TAG, "DirectoryPath is %s\n", uri.c_str());
                    rep->setBaseUrl(uri.append("/"));
                }
            }
        }

        if (bwAttr) {
            rep->setBandwidth(bwAttr->decimal());
        }

        if (resAttr) {
            std::pair<int, int> res = resAttr->getResolution();

            if (res.first && res.second) {
                rep->setWidth(res.first);
                rep->setHeight(res.second);
            }
        }

        if (codecAttr) {
            AF_LOGD("codec is %s\n", codecAttr->value.c_str());
            bool mixedVideo = false;
            bool mixedAudio = false;

            if ((codecAttr->value.find("avc") != std::string::npos
                    || codecAttr->value.find("hvc") != std::string::npos)
                    && !videoAttr) {
                mixedVideo = true;
            }

            if ((codecAttr->value.find("mp4a") != std::string::npos
                    || codecAttr->value.find("ac-3") != std::string::npos
                    || codecAttr->value.find("ec-3") != std::string::npos
                )
                    && !audioAttr) {
                mixedAudio = true;
            }

            if (mixedVideo && mixedAudio) {
                rep->mStreamType = STREAM_TYPE_MIXED;
            } else {
                if (mixedVideo) {
                    rep->mStreamType = STREAM_TYPE_VIDEO;
                } else if (mixedAudio) {
                    rep->mStreamType = STREAM_TYPE_AUDIO;
                } else {
                    rep->mStreamType = STREAM_TYPE_UNKNOWN;
                }
            }
        }

        return rep;
    }

    typedef int64_t mtime_t;

    void HlsParser::parseSegments(dataSourceIO *stream, Representation *rep, const std::list<Tag *> &tagslist)
    {
        auto *segmentList = new SegmentList(rep);
        (void) stream;
        //   rep->setTimescale(100);
        //   rep->b_loaded = true;
        rep->b_live = true;
        mtime_t totalduration = 0;
        mtime_t nzStartTime = 0;
        mtime_t absReferenceTime = 0;
        uint64_t sequenceNumber = 0;
        uint64_t discontinuityNum = 0;
        std::size_t prevbyterangeoffset = 0;
        const SingleValueTag *ctx_byterange = nullptr;
        SegmentEncryption encryption;
        const ValuesListTag *ctx_extinf = nullptr;
        std::list<Tag *>::const_iterator it;
        std::shared_ptr<segment> curInitSegment = nullptr;

        for (it = tagslist.begin(); it != tagslist.end(); ++it) {
            const Tag *tag = *it;

            switch (tag->getType()) {
                /* using static cast as attribute type permits avoiding class check */
                case SingleValueTag::EXTXMEDIASEQUENCE: {
                    sequenceNumber = (static_cast<const SingleValueTag *>(tag))->getValue().decimal();
                }
                break;

                case ValuesListTag::EXTINF: {
                    ctx_extinf = static_cast<const ValuesListTag *>(tag);
                }
                break;

                case SingleValueTag::URI: {
                    const auto *uritag = static_cast<const SingleValueTag *>(tag);

                    if (uritag->getValue().value.empty()) {
                        ctx_extinf = nullptr;
                        ctx_byterange = nullptr;
                        break;
                    }

                    auto pSegment = std::make_shared<segment>(sequenceNumber++);
                    pSegment->setSourceUrl(uritag->getValue().value);
//                    if ((unsigned) rep->getStreamFormat() == StreamFormat::UNKNOWN)
//                        setFormatFromExtension(rep, uritag->getValue().value);
                    /* Need to use EXTXTARGETDURATION as default as some can't properly set segment one */
                    double duration = rep->targetDuration;

                    if (ctx_extinf) {
                        const Attribute *durAttribute = ctx_extinf->getAttributeByName("DURATION");

                        if (durAttribute) {
                            duration = durAttribute->floatingPoint();
                        }

                        ctx_extinf = nullptr;
                    }

                    const auto nzDuration = static_cast<const mtime_t>(CLOCK_FREQ * duration);
                    pSegment->duration = nzDuration;
                    pSegment->startTime = static_cast<uint64_t>(nzStartTime);
                    nzStartTime += nzDuration;
                    totalduration += nzDuration;
//                    if (absReferenceTime > VLC_TS_INVALID) {
//                        segment->utcTime = absReferenceTime;
//                        absReferenceTime += nzDuration;
//                    }
                    pSegment->init_section = curInitSegment;
                    segmentList->addSegment(pSegment);

                    if (ctx_byterange) {
                        std::pair<std::size_t, std::size_t> range = ctx_byterange->getValue().getByteRange();

                        if (range.first == 0) { /* first == size, second = offset */
                            range.first = prevbyterangeoffset;
                        }

                        prevbyterangeoffset = range.first + range.second;
                        pSegment->setByteRange(range.first, prevbyterangeoffset - 1);
                        ctx_byterange = nullptr;
                    }

                    pSegment->discontinuityNum = discontinuityNum;

                    if (encryption.method != SegmentEncryption::NONE) {
                        pSegment->setEncryption(encryption);
                    }
                }
                break;

                case SingleValueTag::EXTXTARGETDURATION:
                    rep->targetDuration = static_cast<const SingleValueTag *>(tag)->getValue().decimal();
                    break;

                case SingleValueTag::EXTXPLAYLISTTYPE:
                    rep->b_live = (static_cast<const SingleValueTag *>(tag)->getValue().value != "VOD");
                    break;

                case SingleValueTag::EXTXBYTERANGE:
                    ctx_byterange = static_cast<const SingleValueTag *>(tag);
                    break;

                case SingleValueTag::EXTXPROGRAMDATETIME:
//                    rep->b_consistent = false;
//                    absReferenceTime = VLC_TS_0 +
//                                       UTCTime(static_cast<const SingleValueTag *>(tag)->getValue().value).mtime();
                    break;

                case AttributesTag::EXTXKEY: {
                    const auto *keytag = static_cast<const AttributesTag *>(tag);

                    if (keytag->getAttributeByName("METHOD") &&
                            keytag->getAttributeByName("METHOD")->value == "AES-128" &&
                            keytag->getAttributeByName("URI")) {
                        encryption.method = SegmentEncryption::AES_128;
                        encryption.iv.clear();
                        encryption.keyUrl = keytag->getAttributeByName("URI")->quotedString();

                        if (keytag->getAttributeByName("IV")) {
                            encryption.iv.clear();
                            encryption.iv = keytag->getAttributeByName("IV")->hexSequence();
                            encryption.ivStatic = true;
                        }
                    } else if (keytag->getAttributeByName("METHOD") &&
                               keytag->getAttributeByName("METHOD")->value == "AES-PRIVATE" &&
                               keytag->getAttributeByName("DATE")) {
                        encryption.method = SegmentEncryption::AES_PRIVATE;
                        encryption.iv.clear();
                        encryption.keyUrl = keytag->getAttributeByName("DATE")->quotedString();

                        if (keytag->getAttributeByName("IV")) {
                            encryption.iv.clear();
                            encryption.iv = keytag->getAttributeByName("IV")->hexSequence();
                        }
                    } else if (keytag->getAttributeByName("METHOD") &&
                               keytag->getAttributeByName("METHOD")->value == "SAMPLE-AES" &&
                               keytag->getAttributeByName("URI")) {
                        encryption.method = SegmentEncryption::AES_SAMPLE;
                        encryption.iv.clear();
                        encryption.keyUrl = keytag->getAttributeByName("URI")->quotedString();

                        if (keytag->getAttributeByName("IV")) {
                            encryption.iv.clear();
                            encryption.iv = keytag->getAttributeByName("IV")->hexSequence();
                            encryption.ivStatic = true;
                        }

                        if (keytag->getAttributeByName("KEYFORMAT")) {
                            encryption.keyFormat = keytag->getAttributeByName("KEYFORMAT")->quotedString();
                        }
                    } else {
                        /* unsupported or invalid */
                        encryption.method = SegmentEncryption::NONE;
                        encryption.keyUrl = "";
                        encryption.iv.clear();
                    }
                }
                break;

                case AttributesTag::EXTXMAP: {
                    const AttributesTag *keytag = static_cast<const AttributesTag *>(tag);
                    const Attribute *uriAttr;

                    if (keytag && (uriAttr = keytag->getAttributeByName("URI"))) {
                        curInitSegment = std::make_shared<segment>(sequenceNumber++);

                        if (curInitSegment) {
                            curInitSegment->setSourceUrl(uriAttr->quotedString());
                            const Attribute *byterangeAttr = keytag->getAttributeByName("BYTERANGE");

                            if (byterangeAttr) {
                                const std::pair<std::size_t, std::size_t> range = byterangeAttr->unescapeQuotes().getByteRange();
                                //   initSegment->setByteRange(range.first, range.first + range.second - 1);
                            }

                            segmentList->addInitSegment(curInitSegment);
                        }
                    }
                }
                break;

                case Tag::EXTXDISCONTINUITY:
                    discontinuityNum++;
                    break;

                case Tag::EXTXENDLIST:
                    rep->b_live = false;
                    break;

                default:
                    break;
            }
        }

        if (rep->b_live) {
            rep->getPlaylist()->setDuration(0);
        } else if (totalduration > rep->getPlaylist()->getDuration()) {
            rep->getPlaylist()->setDuration(totalduration);
        }

//        rep->appendSegmentList(segmentList, true);
        rep->SetSegmentList(segmentList);
    }

    void HlsParser::createAndFillRepresentation(dataSourceIO *stream, AdaptationSet *adaptSet,
            const AttributesTag *tag,
            const std::list<Tag *> &tagslist)
    {
        Representation *rep = createRepresentation(adaptSet, tag);

        if (rep) {
            parseSegments(stream, rep, tagslist);

            if (rep->b_live) {
                /* avoid update playlist immediately */
//                uint64_t startseq = rep->getLiveStartSegmentNumber(0);
//                rep->scheduleNextUpdate(startseq);
            }

            adaptSet->addRepresentation(rep);
            rep->mStreamType = STREAM_TYPE_MIXED;
        }
    }

    static void releaseTagsList(std::list<Tag *> &list)
    {
        std::list<Tag *>::const_iterator it;

        for (it = list.begin(); it != list.end(); ++it) {
            delete *it;
        }

        list.clear();
    }

    playList *HlsParser::parse(const std::string &playlisturl)
    {
        if (mDataSourceIO == nullptr) {
            mDataSourceIO = new dataSourceIO(mReadCb, mSeekCb, mCBArg);
        }

        int ret = mDataSourceIO->get_line(mBuffer, MAX_LINE_SIZE);

        if (ret < 0 || strncmp(mBuffer, "#EXTM3U", 7) != 0 ||
                (mBuffer[7] && !AfString::isSpace(mBuffer[7]))) {
            AF_LOGE("can't detected a hls playList");
            return nullptr;
        }

        auto *playlist = new playList();

        if (!playlisturl.empty()) {
            playlist->setPlaylistUrl(Helper::getDirectoryPath(playlisturl).append("/"));
        }

        auto *period = new Period(playlist);
        std::list<Tag *> tagsList = parseEntries(mDataSourceIO);
        bool b_masterPlaylist = !getTagsFromList(tagsList, AttributesTag::EXTXSTREAMINF).empty();

        if (b_masterPlaylist) {
            AF_LOGD("MasterPlaylist\n");
            std::list<Tag *>::const_iterator it;
            std::map<std::string, AttributesTag *> groupsMap;
            /* We'll need to create an adaptation set for each media group / alternative rendering
             * we create a list of playlist being and alternative/group */
            std::list<Tag *> mediaInfoTags = getTagsFromList(tagsList, AttributesTag::EXTXMEDIA);
            std::list<AttributesTag *> dummyMediaInfoTags{};

            for (it = mediaInfoTags.begin(); it != mediaInfoTags.end(); ++it) {
                auto *tag = dynamic_cast<AttributesTag *>(*it);

                if (tag) {
                    if (tag->getAttributeByName("URI")) {
                        std::pair<std::string, AttributesTag *> pair(tag->getAttributeByName("URI")->quotedString(), tag);
                        groupsMap.insert(pair);
                    } else {
                        dummyMediaInfoTags.push_back(tag);
                    }
                }
            }

            auto *adaptSet = new AdaptationSet(period);
            std::list<Tag *> streamInfoTags = getTagsFromList(tagsList, AttributesTag::EXTXSTREAMINF);

            for (it = streamInfoTags.begin(); it != streamInfoTags.end(); ++it) {
                auto *tag = dynamic_cast<AttributesTag *>(*it);

                if (tag && tag->getAttributeByName("URI")) {
                    if (groupsMap.find(tag->getAttributeByName("URI")->value) == groupsMap.end()) {
                        /* not a group, belong to default adaptation set */
                        auto *represent = createRepresentation(adaptSet, tag);

                        if (represent->mStreamType != STREAM_TYPE_MIXED && !dummyMediaInfoTags.empty()) {
                            const Attribute *videoAttr = tag->getAttributeByName("VIDEO");
                            const Attribute *audioAttr = tag->getAttributeByName("AUDIO");

                            for (auto item : dummyMediaInfoTags) {
                                const Attribute *group = item->getAttributeByName("GROUP-ID");

                                if (!group || group->value.empty()) {
                                    continue;
                                }

                                AF_LOGD("group name is %s\n", group->value.c_str());

                                if ((videoAttr && group->value == videoAttr->value && represent->mStreamType != STREAM_TYPE_VIDEO) ||
                                        (audioAttr && group->value == audioAttr->value && represent->mStreamType != STREAM_TYPE_AUDIO)) {
                                    if (represent->mStreamType == STREAM_TYPE_UNKNOWN) {
                                        if (audioAttr) {
                                            represent->mStreamType = STREAM_TYPE_AUDIO;
                                        } else {
                                            represent->mStreamType = STREAM_TYPE_VIDEO;
                                        }
                                    } else {
                                        represent->mStreamType = STREAM_TYPE_MIXED;
                                    }

                                    break;
                                }
                            }
                        }

                        adaptSet->addRepresentation(represent);
                    }
                }
            }

            if (!adaptSet->getRepresentations().empty()) {
                period->addAdaptationSet(adaptSet);
            } else {
                delete adaptSet;
            }

//            unsigned set_id = 1;
            std::map<std::string, AttributesTag *>::const_iterator groupsit;

            for (groupsit = groupsMap.begin(); groupsit != groupsMap.end(); ++groupsit) {
                auto *altAdaptSet = new AdaptationSet(period);
                std::pair<std::string, AttributesTag *> pair = *groupsit;
                Representation *rep = createRepresentation(altAdaptSet, pair.second);
                altAdaptSet->addRepresentation(rep);
                std::string desc;

                if (pair.second->getAttributeByName("GROUP-ID")) {
                    desc = pair.second->getAttributeByName("GROUP-ID")->quotedString();
                }

                if (pair.second->getAttributeByName("NAME")) {
                    if (!desc.empty()) {
                        desc += " ";
                    }

                    desc += pair.second->getAttributeByName("NAME")->quotedString();
                }

//               AF_LOGD(TAG, "desc is %s\n", desc.c_str());

                if (!desc.empty()) {
                    altAdaptSet->setDescription(desc);
                    //altAdaptSet->setID(ID(desc));
                }

//                else
//                    altAdaptSet->setID(ID(set_id++));

//                /* Subtitles unsupported for now */
//                if (pair.second->getAttributeByName("TYPE")->value != "AUDIO" &&
//                    pair.second->getAttributeByName("TYPE")->value != "VIDEO" &&
//                    pair.second->getAttributeByName("TYPE")->value != "SUBTITLES") {
//                    //rep->streamFormat = StreamFormat(StreamFormat::UNSUPPORTED);
//                }
                if (pair.second->getAttributeByName("TYPE")->value == "AUDIO") {
                    rep->mStreamType = STREAM_TYPE_AUDIO;
                } else if (pair.second->getAttributeByName("TYPE")->value == "VIDEO") {
                    rep->mStreamType = STREAM_TYPE_VIDEO;
                } else if (pair.second->getAttributeByName("TYPE")->value == "SUBTITLES") {
                    rep->mStreamType = STREAM_TYPE_SUB;
                } else {
                    rep->mStreamType = STREAM_TYPE_MIXED;
                }

                if (pair.second->getAttributeByName("LANGUAGE")) {
                    std::string lang = pair.second->getAttributeByName("LANGUAGE")->quotedString();
                    std::size_t pos = lang.find_first_of('-');

                    if (pos != std::string::npos && pos > 0) {
                        //altAdaptSet->addLang(lang.substr(0, pos));
                        rep->mLang = lang.substr(0, pos);
                    } else if (lang.size() < 4) {
                        //altAdaptSet->addLang(lang);
                        rep->mLang = lang;
                    }
                }

                if (!altAdaptSet->getRepresentations().empty()) {
                    period->addAdaptationSet(altAdaptSet);
                } else {
                    delete altAdaptSet;
                }
            }
        } else {/* Non master playlist (opened directly subplaylist or HLS v1) */
            auto *adaptSet = new AdaptationSet(period);
            period->addAdaptationSet(adaptSet);
            AttributesTag *tag = new AttributesTag(AttributesTag::EXTXSTREAMINF, "");
            tag->addAttribute(new Attribute("URI", playlisturl));
            createAndFillRepresentation(mDataSourceIO, adaptSet, tag, tagsList);
            delete tag;
        }

        playlist->addPeriod(period);
        releaseTagsList(tagsList);
        return playlist;
    }

    std::list<Tag *> HlsParser::parseEntries(dataSourceIO *stream)
    {
        std::list<Tag *> entrieslist;
        Tag *lastTag = nullptr;

        while (!stream->isEOF()) {
            stream->get_line(mBuffer, MAX_LINE_SIZE);

            if (*mBuffer == '#') {
                if (!strncmp(mBuffer, "#EXT", 4)) { //tag
                    std::string key;
                    std::string attributes;
                    const char *split = strchr(mBuffer, ':');

                    if (split) {
                        key = std::string(mBuffer + 1, split - mBuffer - 1);
                        attributes = std::string(split + 1);
                    } else {
                        key = std::string(mBuffer + 1);
                    }

                    if (!key.empty()) {
                        Tag *tag = TagFactory::createTagByName(key, attributes);

                        if (tag) {
                            entrieslist.push_back(tag);
                        }

                        lastTag = tag;
                    }
                }
            } else if (*mBuffer) {
                /* URI */
                if (lastTag && lastTag->getType() == AttributesTag::EXTXSTREAMINF) {
                    auto *streaminftag = static_cast<AttributesTag *>(lastTag);
                    /* master playlist uri, merge as attribute */
                    Attribute *uriAttr = new (std::nothrow) Attribute("URI", std::string(mBuffer));

                    if (uriAttr) {
                        streaminftag->addAttribute(uriAttr);
                    }
                } else {/* playlist tag, will take modifiers */
                    Tag *tag = TagFactory::createTagByName("", std::string(mBuffer));

                    if (tag) {
                        entrieslist.push_back(tag);
                    }
                }

                lastTag = nullptr;
            } else {// drop
                lastTag = nullptr;
            }
        }

        return entrieslist;
    }
}
