//
// Created by yuyuan on 2021/03/08.
//

#include "MPDParser.h"
#include "ProgramInfo.h"
#include "SegmentBase.h"
#include "SegmentList.h"
#include "SegmentTemplate.h"
#include "SegmentTimeline.h"
#include "UTCTiming.h"
#include "conversions.h"
#include "data_source/dataSourceIO.h"
#include "demuxer/play_list/AdaptationSet.h"
#include "demuxer/play_list/Helper.h"
#include "demuxer/play_list/Period.h"
#include "demuxer/play_list/Representation.h"
#include "utils/xml/DOMHelper.h"
#include "utils/xml/DOMParser.h"
#include <limits>
#include <locale>

using namespace Cicada;
using namespace Cicada::Dash;

MPDParser::MPDParser(std::string url) : playlisturl(std::move(url))
{}

MPDParser::~MPDParser()
{}

Cicada::playList *MPDParser::parse(const std::string &playlistur)
{
    if (mDataSourceIO == nullptr) {
        mDataSourceIO = new dataSourceIO(mReadCb, mSeekCb, mCBArg);
    }

    int64_t size = 0;
    int64_t buffer_size = 128;
    char *buffer = (char *) malloc(buffer_size);
    while (!mDataSourceIO->isEOF()) {
        char c = mDataSourceIO->readChar();
        size++;
        if (size > buffer_size) {
            buffer_size = buffer_size * 2;
            buffer = (char *) realloc(buffer, buffer_size);
        }
        buffer[size - 1] = c;
    }

    DOMParser domParser;
    domParser.parse((const char *) buffer, size);
    mRoot = domParser.getRootNode();
    if (mRoot == nullptr) {
        return nullptr;
    }

    auto *mpd = new MPDPlayList();
    if (!playlisturl.empty()) {
        mpd->setPlaylistUrl(Helper::getDirectoryPath(playlisturl).append("/"));
    }
    parseMPDAttributes(mpd, mRoot);
    parseProgramInformation(mpd, DOMHelper::getFirstChildElementByName(mRoot, "ProgramInformation"));
    parseUtcTiming(mpd, DOMHelper::getFirstChildElementByName(mRoot, "UTCTiming"));
    parseMPDBaseUrl(mpd, mRoot);
    parsePeriods(mpd, mRoot);
    mpd->InitUtcTime();
    mRoot = nullptr;
    free(buffer);
    return mpd;
}

int MPDParser::probe(const uint8_t *buffer, int size)
{
    std::string lower_buffer;
    for (size_t i = 0; i < size; i++) {
        lower_buffer += (char) tolower(buffer[i]);
    }

    if (!strstr(lower_buffer.c_str(), "<mpd")) {
        return 0;
    }

    if (strstr(lower_buffer.c_str(), "dash:profile:isoff-on-demand:2011") || strstr(lower_buffer.c_str(), "dash:profile:isoff-live:2011") ||
        strstr(lower_buffer.c_str(), "dash:profile:isoff-live:2012") || strstr(lower_buffer.c_str(), "dash:profile:isoff-main:2011") ||
        strstr(lower_buffer.c_str(), "3gpp:pss:profile:dash1")) {
        return 100;
    }
    if (strstr(lower_buffer.c_str(), "dash:profile")) {
        return 100;
    }

    return 0;
}

void MPDParser::parseMPDAttributes(MPDPlayList *mpd, xml::Node *node)
{
    const std::map<std::string, std::string> &attr = node->getAttributes();

    std::map<std::string, std::string>::const_iterator it;
    it = attr.find("mediaPresentationDuration");
    if (it != attr.end()) {
        mpd->setDuration(IsoTimeGetUs(it->second));
    }

    it = attr.find("minBufferTime");
    if (it != attr.end()) {
        mpd->minBufferTime = IsoTimeGetUs(it->second);
    }

    it = attr.find("minimumUpdatePeriod");
    if (it != attr.end()) {
        mpd->needsUpdates = true;
        int64_t minupdate = IsoTimeGetUs(it->second);
        if (minupdate > 0) {
            mpd->minUpdatePeriod = minupdate;
        }
    } else {
        mpd->needsUpdates = false;
    }

    it = attr.find("maxSegmentDuration");
    if (it != attr.end()) {
        mpd->maxSegmentDuration = IsoTimeGetUs(it->second);
    }

    it = attr.find("type");
    if (it != attr.end()) {
        mpd->type = it->second;
    }

    it = attr.find("profiles");
    if (it == attr.end()) {
        it = attr.find("profile");
    }
    if (it != attr.end()) {
        std::string urn = it->second;
        mpd->setProfile(urn);
    }

    it = attr.find("availabilityStartTime");
    if (it != attr.end()) {
        mpd->availabilityStartTime = UTCTimeGetUS(it->second);
    }

    it = attr.find("availabilityEndTime");
    if (it != attr.end()) {
        mpd->availabilityEndTime = UTCTimeGetUS(it->second);
    }

    it = attr.find("timeShiftBufferDepth");
    if (it != attr.end()) {
        mpd->timeShiftBufferDepth = IsoTimeGetUs(it->second);
    }

    it = attr.find("suggestedPresentationDelay");
    if (it != attr.end()) {
        mpd->suggestedPresentationDelay = IsoTimeGetUs(it->second);
    }
}

void MPDParser::parseProgramInformation(MPDPlayList *mpd, xml::Node *node)
{
    if (!node) {
        return;
    }

    auto *info = new ProgramInfo();
    xml::Node *child = DOMHelper::getFirstChildElementByName(node, "Title");
    if (child) {
        info->title = child->getText();
    }

    child = DOMHelper::getFirstChildElementByName(node, "Source");
    if (child) {
        info->source = child->getText();
    }

    child = DOMHelper::getFirstChildElementByName(node, "Copyright");
    if (child) {
        info->copyright = child->getText();
    }

    if (node->hasAttribute("moreInformationURL")) {
        info->moreInformationUrl = node->getAttributeValue("moreInformationURL");
    }

    mpd->programInfo = static_cast<unique_ptr<ProgramInfo>>(info);
}

void MPDParser::parseUtcTiming(MPDPlayList *mpd, xml::Node *node)
{
    if (!node) {
        return;
    }

    std::string schemeIdUri, value;
    if (node->hasAttribute("schemeIdUri")) {
        schemeIdUri = node->getAttributeValue("schemeIdUri");
    }
    if (node->hasAttribute("value")) {
        value = node->getAttributeValue("value");
    }
    mpd->mUtcTiming = new UTCTiming(schemeIdUri, value);
}

void MPDParser::parseMPDBaseUrl(MPDPlayList *mpd, xml::Node *root)
{
    std::vector<xml::Node *> baseUrls = DOMHelper::getChildElementByTagName(root, "BaseURL");

    for (auto &baseUrl : baseUrls) {
        mpd->addBaseUrl(baseUrl->getText());
    }

    mpd->setPlaylistUrl(Helper::getDirectoryPath(playlisturl).append("/"));
}

template<class T>
static void parseAvailability(MPDPlayList *mpd, xml::Node *node, T *s)
{
    if (node->hasAttribute("availabilityTimeOffset")) {
        double val = std::stod(node->getAttributeValue("availabilityTimeOffset"));
        s->addAttribute(new AvailabilityTimeOffsetAttr(val * 1000000));
    }
    if (node->hasAttribute("availabilityTimeComplete")) {
        bool b = (node->getAttributeValue("availabilityTimeComplete") == "false");
        s->addAttribute(new AvailabilityTimeCompleteAttr(!b));
        if (b) {
            mpd->setLowLatency(b);
        }
    }
}

void MPDParser::parsePeriods(MPDPlayList *mpd, xml::Node *root)
{
    std::vector<xml::Node *> periods = DOMHelper::getElementByTagName(root, "Period", false);
    std::vector<xml::Node *>::const_iterator it;
    uint64_t nextid = 0;

    for (it = periods.begin(); it != periods.end(); ++it) {
        Period *period = new (std::nothrow) Period(mpd);
        if (!period) {
            continue;
        }
        parseSegmentInformation(mpd, *it, period, &nextid);
        if ((*it)->hasAttribute("start")) {
            period->startTime = IsoTimeGetUs((*it)->getAttributeValue("start"));
        }
        if ((*it)->hasAttribute("duration")) {
            period->duration = IsoTimeGetUs((*it)->getAttributeValue("duration"));
        }
        parseBaseUrl(mpd, *it, period);
        parseAdaptationSets(mpd, *it, period);
        mpd->addPeriod(period);
    }
}

size_t MPDParser::parseSegmentInformation(MPDPlayList *mpd, xml::Node *node, SegmentInformation *info, uint64_t *nextid)
{
    size_t total = 0;
    total += parseSegmentBase(mpd, DOMHelper::getFirstChildElementByName(node, "SegmentBase"), info);
    total += parseSegmentList(mpd, DOMHelper::getFirstChildElementByName(node, "SegmentList"), info);
    total += parseSegmentTemplate(mpd, DOMHelper::getFirstChildElementByName(node, "SegmentTemplate"), info);

    if (node->hasAttribute("timescale")) {
        std::string str_timescale = node->getAttributeValue("timescale");
        uint64_t ull_timescale = std::stoull(str_timescale);
        info->addAttribute(new TimescaleAttr(Timescale(ull_timescale)));
    }

    parseAvailability<SegmentInformation>(mpd, node, info);

    if (node->hasAttribute("id")) {
        info->setID(node->getAttributeValue("id"));
    } else {
        info->setID(to_string((*nextid)++));
    }

    return total;
}

size_t MPDParser::parseSegmentBase(MPDPlayList *mpd, xml::Node *segmentBaseNode, SegmentInformation *info)
{
    if (segmentBaseNode == nullptr) {
        return 0;
    }
    SegmentBase *base = new SegmentBase(info);
    if (base == nullptr) {
        return 0;
    }

    parseCommonSegmentBase(mpd, segmentBaseNode, base, info);

    parseAvailability<SegmentInformation>(mpd, segmentBaseNode, info);

    if (!base->getInitSegment() && base->getIndexSegment() && base->getIndexSegment()->getOffset()) {
        DashSegment *initSeg = new DashSegment(info);
        initSeg->setSourceUrl(base->getUrlSegment().toString());
        initSeg->setByteRange(0, base->getIndexSegment()->getOffset() - 1);
        initSeg->isInitSegment = true;
        base->setInitSegment(initSeg);
    }

    info->addAttribute(base);

    return 1;
}

void MPDParser::parseAdaptationSets(MPDPlayList *mpd, xml::Node *periodNode, Period *period)
{
    std::vector<xml::Node *> adaptationSets = DOMHelper::getElementByTagName(periodNode, "AdaptationSet", false);
    std::vector<xml::Node *>::const_iterator it;
    uint64_t nextid = 0;

    for (it = adaptationSets.begin(); it != adaptationSets.end(); ++it) {
        auto *adaptationSet = new AdaptationSet(period);
        if ((*it)->hasAttribute("mimeType")) {
            adaptationSet->mimeType = (*it)->getAttributeValue("mimeType");
        }

        if ((*it)->hasAttribute("lang")) {
            std::string lang = (*it)->getAttributeValue("lang");
            std::size_t pos = adaptationSet->lang.find_first_of('-');
            if (pos != std::string::npos && pos > 0) {
                adaptationSet->lang = lang.substr(0, pos);
            } else if (lang.size() < 4) {
                adaptationSet->lang = lang;
            }
        }

        if ((*it)->hasAttribute("bitstreamSwitching")) {
            adaptationSet->bitswitchAble = ((*it)->getAttributeValue("bitstreamSwitching") == "true");
        }

        if ((*it)->hasAttribute("segmentAlignment")) {
            adaptationSet->segmentAligned = ((*it)->getAttributeValue("segmentAlignment") == "true");
        }
        parseBaseUrl(mpd, *it, adaptationSet);

        parseSegmentInformation(mpd, *it, adaptationSet, &nextid);

        parseRepresentations(mpd, (*it), adaptationSet);

        if (!adaptationSet->getRepresentations().empty()) {
            period->addAdaptationSet(adaptationSet);
        } else {
            delete adaptationSet;
        }
    }
}

void MPDParser::parseRepresentations(MPDPlayList *mpd, xml::Node *adaptationSetNode, AdaptationSet *adaptationSet)
{
    std::vector<xml::Node *> representations = DOMHelper::getElementByTagName(adaptationSetNode, "Representation", false);
    uint64_t nextid = 0;

    for (auto repNode : representations) {
        auto *currentRepresentation = new Representation(adaptationSet);

        parseBaseUrl(mpd, repNode, currentRepresentation);

        if (repNode->hasAttribute("id")) {
            currentRepresentation->setID(repNode->getAttributeValue("id"));
        }

        if (repNode->hasAttribute("width")) {
            currentRepresentation->setWidth(std::stoi(repNode->getAttributeValue("width")));
        }

        if (repNode->hasAttribute("height")) {
            currentRepresentation->setHeight(std::stoi(repNode->getAttributeValue("height")));
        }

        if (repNode->hasAttribute("bandwidth")) {
            currentRepresentation->setBandwidth(std::stoi(repNode->getAttributeValue("bandwidth")));
        }

        if (repNode->hasAttribute("mimeType")) {
            currentRepresentation->mimeType = repNode->getAttributeValue("mimeType");
            currentRepresentation->updateStreamType();
        }

        if (repNode->hasAttribute("codecs")) {
            currentRepresentation->addCodecs(repNode->getAttributeValue("codecs"));
        }
        size_t i_total = parseSegmentInformation(mpd, repNode, currentRepresentation, &nextid);

        // Empty Representation with just baseurl (ex: subtitles)
        if (i_total == 0 && (currentRepresentation->baseUrl && !currentRepresentation->baseUrl->empty()) &&
            adaptationSet->getMediaSegment(0) == nullptr) {
            SegmentBase *base = new (std::nothrow) SegmentBase(currentRepresentation);
            if (base) {
                currentRepresentation->addAttribute(base);
            }
        }

        adaptationSet->addRepresentation(currentRepresentation);
    }
}

size_t MPDParser::parseSegmentList(MPDPlayList *mpd, xml::Node *segListNode, SegmentInformation *info)
{
    size_t total = 0;
    if (segListNode) {
        std::vector<xml::Node *> segments = DOMHelper::getElementByTagName(segListNode, "SegmentURL", false);
        Dash::SegmentList *list;
        if ((list = new (std::nothrow) Dash::SegmentList(info))) {
            parseCommonMultiSegmentBase(mpd, segListNode, list, info);

            parseAvailability<SegmentInformation>(mpd, segListNode, info);

            uint64_t nzStartTime = 0;
            std::vector<xml::Node *>::const_iterator it;
            for (it = segments.begin(); it != segments.end(); ++it) {
                xml::Node *segmentURL = *it;

                DashSegment *seg = new (std::nothrow) DashSegment(info);
                if (!seg) {
                    continue;
                }

                std::string mediaUrl = segmentURL->getAttributeValue("media");
                if (!mediaUrl.empty()) {
                    seg->setSourceUrl(mediaUrl);
                }

                if (segmentURL->hasAttribute("mediaRange")) {
                    std::string range = segmentURL->getAttributeValue("mediaRange");
                    size_t pos = range.find("-");
                    seg->setByteRange(std::stoi(range.substr(0, pos)), std::stoi(range.substr(pos + 1, range.size())));
                }

                int64_t duration = list->inheritDuration();
                if (duration) {
                    seg->startTime = nzStartTime;
                    seg->duration = duration;
                    nzStartTime += duration;
                }

                seg->setSequenceNumber(total);

                list->addSegment(seg);
                total++;
            }

            info->updateSegmentList(list, true);
        }
    }
    return total;
}

size_t MPDParser::parseSegmentTemplate(MPDPlayList *mpd, xml::Node *templateNode, SegmentInformation *info)
{
    size_t total = 0;
    if (templateNode == nullptr) {
        return total;
    }

    std::string mediaurl;
    if (templateNode->hasAttribute("media")) {
        mediaurl = templateNode->getAttributeValue("media");
    }

    SegmentTemplate *mediaTemplate = new SegmentTemplate(new SegmentTemplateSegment(), info);
    if (!mediaTemplate) {
        return total;
    }
    mediaTemplate->setSourceUrl(mediaurl);

    parseCommonMultiSegmentBase(mpd, templateNode, mediaTemplate, info);

    parseAvailability<SegmentInformation>(mpd, templateNode, info);

    if (templateNode->hasAttribute("initialization")) /* /!\ != Initialization */
    {
        SegmentTemplateInit *initTemplate;
        std::string initurl = templateNode->getAttributeValue("initialization");
        if (!initurl.empty() && (initTemplate = new (std::nothrow) SegmentTemplateInit(mediaTemplate, info))) {
            initTemplate->setSourceUrl(initurl);
            delete mediaTemplate->getInitSegment();
            initTemplate->isInitSegment = true;
            mediaTemplate->setInitSegment(initTemplate);
        }
    }

    info->setSegmentTemplate(mediaTemplate);

    return mediaurl.empty() ? ++total : 0;
}

void MPDParser::parseTimeline(MPDPlayList *mpd, xml::Node *node, ISegmentBase *base)
{
    if (!node) {
        return;
    }

    uint64_t number = 0;
    if (node->hasAttribute("startNumber")) {
        number = std::stoull(node->getAttributeValue("startNumber"));
    } else if (base->inheritStartNumber()) {
        number = base->inheritStartNumber();
    }

    SegmentTimeline *timeline = new (std::nothrow) SegmentTimeline(base);
    if (timeline) {
        std::vector<xml::Node *> elements = DOMHelper::getElementByTagName(node, "S", false);
        std::vector<xml::Node *>::const_iterator it;
        for (it = elements.begin(); it != elements.end(); ++it) {
            const xml::Node *s = *it;
            if (!s->hasAttribute("d")) /* Mandatory */
            {
                continue;
            }
            int64_t d = std::stoll(s->getAttributeValue("d"));
            int64_t r = 0;// never repeats by default
            if (s->hasAttribute("r")) {
                r = std::stoll(s->getAttributeValue("r"));
                if (r < 0) {
                    r = std::numeric_limits<unsigned>::max();
                }
            }

            if (s->hasAttribute("t")) {
                int64_t t = std::stoll(s->getAttributeValue("t"));
                timeline->addElement(number, d, r, t);
            } else {
                timeline->addElement(number, d, r);
            }

            number += (1 + r);
        }
        base->addAttribute(timeline);
    }
}

void MPDParser::parseCommonSegmentBase(MPDPlayList *mpd, xml::Node *node, ISegmentBase *base, SegmentInformation *parent)
{
    parseInitSegment(DOMHelper::getFirstChildElementByName(node, "Initialization"), base, parent);

    if (node->hasAttribute("indexRange")) {
        size_t start = 0, end = 0;
        if (std::sscanf(node->getAttributeValue("indexRange").c_str(), "%zu-%zu", &start, &end) == 2) {
            DashSegment *index = new DashSegment(parent);
            if (index) {
                index->setByteRange(start, end);
                base->setIndexSegment(index);
                /* index must be before data, so data starts at index end */
                SegmentBase *segBase = dynamic_cast<SegmentBase *>(base);
                if (segBase) {
                    segBase->setByteRange(end + 1, INT64_MIN);
                    DashSegment *subSeg = new DashSegment(parent, end + 1, INT64_MIN);
                    segBase->addSubSegment(subSeg);
                }
            }
        }
    }

    if (node->hasAttribute("timescale")) {
        TimescaleAttr *prop = new TimescaleAttr(Timescale(std::stoull(node->getAttributeValue("timescale"))));
        base->addAttribute(prop);
    }
}

void MPDParser::parseCommonMultiSegmentBase(MPDPlayList *mpd, xml::Node *node, ISegmentBase *base, SegmentInformation *parent)
{
    parseCommonSegmentBase(mpd, node, base, parent);

    if (node->hasAttribute("duration")) {
        base->addAttribute(new DurationAttr(std::stoll(node->getAttributeValue("duration"))));
    }

    if (node->hasAttribute("startNumber")) {
        base->addAttribute(new StartnumberAttr(std::stoull(node->getAttributeValue("startNumber"))));
    }

    parseTimeline(mpd, DOMHelper::getFirstChildElementByName(node, "SegmentTimeline"), base);
}

void MPDParser::parseInitSegment(xml::Node *initNode, ISegmentBase *base, SegmentInformation *parent)
{
    if (!initNode) {
        return;
    }

    DashSegment *seg = new DashSegment(parent);
    seg->setSourceUrl(initNode->getAttributeValue("sourceURL"));

    if (initNode->hasAttribute("range")) {
        std::string range = initNode->getAttributeValue("range");
        size_t pos = range.find("-");
        seg->setByteRange(std::stoi(range.substr(0, pos)), std::stoi(range.substr(pos + 1, range.size())));
    }
    seg->isInitSegment = true;
    base->setInitSegment(seg);
}

void MPDParser::parseBaseUrl(MPDPlayList *mpd, xml::Node *containerNode, SegmentInformation *parent)
{
    xml::Node *baseUrl = DOMHelper::getFirstChildElementByName(containerNode, "BaseURL");
    if (baseUrl) {
        parent->baseUrl = static_cast<unique_ptr<DashUrl>>(new DashUrl(baseUrl->getText()));
        parseAvailability<SegmentInformation>(mpd, baseUrl, parent);
    }
}