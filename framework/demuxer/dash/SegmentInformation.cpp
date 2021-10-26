//
// Created by yuyuan on 2021/03/09.
//

#include "SegmentInformation.h"
#include "ISegmentBase.h"
#include "SegmentBase.h"
#include "SegmentList.h"
#include "SegmentTemplate.h"
#include "demuxer/play_list/Representation.h"
#include "demuxer/play_list/playList.h"
#include <assert.h>

using namespace Cicada;
using namespace Cicada::Dash;

SegmentInformation::SegmentInformation(SegmentInformation *parent) : IDashUrl(parent), AttrsNode(Type::SegmentInformation, parent)
{
    mParent = parent;
}

SegmentInformation::SegmentInformation(playList *parent) : IDashUrl(parent), AttrsNode(AbstractAttr::Type::SegmentInformation, nullptr)
{
    mParent = nullptr;
}

SegmentInformation ::~SegmentInformation()
{}

void SegmentInformation::setID(const std::string &id)
{
    mId = id;
}

std::string SegmentInformation::getID() const
{
    return mId;
}

const ISegmentBase *SegmentInformation::inheritSegmentProfile() const
{
    const ISegmentBase *profile = inheritSegmentTemplate();
    if (!profile) {
        profile = inheritSegmentList();
    }
    if (!profile) {
        profile = inheritSegmentBase();
    }
    return profile;
}

int64_t SegmentInformation::getMediaSegmentStartTime(uint64_t i_pos) const
{
    int64_t startTime = 0;
    int64_t duration = 0;
    const ISegmentBase *profile = inheritSegmentProfile();
    if (profile == nullptr) {
        return startTime;
    }
    profile->getPlaybackTimeDurationBySegmentNumber(i_pos, &startTime, &duration);
    return startTime;
}

/* Returns wanted segment, or next in sequence if not found */
DashSegment *SegmentInformation::getNextMediaSegment(uint64_t i_pos, uint64_t *pi_newpos, bool *pb_gap) const
{
    const ISegmentBase *profile = inheritSegmentProfile();
    if (!profile) {
        return nullptr;
    }
    return profile->getNextMediaSegment(i_pos, pi_newpos, pb_gap);
}

DashSegment *SegmentInformation::getInitSegment() const
{
    const ISegmentBase *profile = inheritSegmentProfile();
    if (!profile) {
        return nullptr;
    }
    return profile->getInitSegment();
}

DashSegment *SegmentInformation::getIndexSegment() const
{
    const ISegmentBase *profile = inheritSegmentProfile();
    if (!profile) {
        return nullptr;
    }
    return profile->getIndexSegment();
}

DashSegment *SegmentInformation::getMediaSegment(uint64_t pos) const
{
    const ISegmentBase *profile = inheritSegmentProfile();
    if (!profile) {
        return nullptr;
    }
    return profile->getMediaSegment(pos);
}

SegmentInformation *SegmentInformation::getChildByID(const std::string &id)
{
    std::vector<SegmentInformation *>::const_iterator it;
    for (it = childs.begin(); it != childs.end(); ++it) {
        if ((*it)->getID() == id) {
            return *it;
        }
    }
    return nullptr;
}

void SegmentInformation::updateWith(SegmentInformation *updated)
{
    /* Support Segment List for now */
    AbstractAttr *p = getAttribute(Type::SegmentList);
    if (p && p->isValid() && updated->getAttribute(Type::SegmentList)) {
        inheritSegmentList()->updateWith(updated->inheritSegmentList());
    }

    p = getAttribute(Type::SegmentTemplate);
    if (p && p->isValid() && updated->getAttribute(Type::SegmentTemplate)) {
        inheritSegmentTemplate()->updateWith(updated->inheritSegmentTemplate());
    }

    std::vector<SegmentInformation *>::const_iterator it;
    for (it = childs.begin(); it != childs.end(); ++it) {
        SegmentInformation *child = *it;
        SegmentInformation *updatedChild = updated->getChildByID(child->getID());
        if (updatedChild) {
            child->updateWith(updatedChild);
        }
    }
    /* FIXME: handle difference */
}

void SegmentInformation::pruneByPlaybackTime(int64_t time)
{
    Dash::SegmentList *segmentList = static_cast<Dash::SegmentList *>(getAttribute(Type::SegmentList));
    if (segmentList) {
        segmentList->pruneByPlaybackTime(time);
    }

    SegmentTemplate *templ = static_cast<SegmentTemplate *>(getAttribute(Type::SegmentTemplate));
    if (templ) {
        templ->pruneByPlaybackTime(time);
    }

    std::vector<SegmentInformation *>::const_iterator it;
    for (it = childs.begin(); it != childs.end(); ++it) {
        (*it)->pruneByPlaybackTime(time);
    }
}

void SegmentInformation::pruneBySegmentNumber(uint64_t num)
{
    assert(dynamic_cast<Representation *>(this));

    Dash::SegmentList *segmentList = static_cast<Dash::SegmentList *>(getAttribute(Type::SegmentList));
    if (segmentList) {
        segmentList->pruneBySegmentNumber(num);
    }

    SegmentTemplate *templ = static_cast<SegmentTemplate *>(getAttribute(Type::SegmentTemplate));
    if (templ) {
        templ->pruneBySequenceNumber(num);
    }
}

int64_t SegmentInformation::getPeriodStart() const
{
    if (mParent) {
        return mParent->getPeriodStart();
    } else {
        return 0;
    }
}

int64_t SegmentInformation::getPeriodDuration() const
{
    if (mParent) {
        return mParent->getPeriodDuration();
    } else {
        return 0;
    }
}

ISegmentBase *SegmentInformation::getProfile() const
{
    AbstractAttr *p;
    if ((p = getAttribute(Type::SegmentTemplate))) {
        return static_cast<SegmentTemplate *>(p);
    } else if ((p = getAttribute(Type::SegmentList))) {
        return static_cast<Dash::SegmentList *>(p);
    } else if ((p = getAttribute(Type::SegmentBase))) {
        return static_cast<SegmentBase *>(p);
    }

    return nullptr;
}

void SegmentInformation::updateSegmentList(Dash::SegmentList *list, bool restamp)
{
    Dash::SegmentList *segmentList = static_cast<Dash::SegmentList *>(getAttribute(Type::SegmentList));
    if (segmentList && restamp) {
        segmentList->updateWith(list, restamp);
        delete list;
    } else {
        replaceAttribute(list);
    }
}

void SegmentInformation::setSegmentTemplate(SegmentTemplate *templ)
{
    SegmentTemplate *local = static_cast<SegmentTemplate *>(getAttribute(Type::SegmentTemplate));
    if (local) {
        local->updateWith(templ);
        delete templ;
    } else {
        addAttribute(templ);
    }
}

static void insertIntoSegment(DashSegment *container, size_t start, size_t end, int64_t time, int64_t duration)
{
    if (end == 0 || container->contains(end)) {
        DashSegment *subsegment = new DashSegment(container, start, (end != 0) ? end : 0);
        subsegment->startTime = time;
        subsegment->duration = duration;
        container->addSubSegment(subsegment);
    }
}

void SegmentInformation::SplitUsingIndex(std::vector<SplitPoint> &splitlist)
{
    DashSegment *segmentBase = inheritSegmentBase();
    if (!segmentBase) {
        return;
    }

    size_t prevstart = 0;
    int64_t prevtime = 0;

    SplitPoint split = {0, 0, 0};
    std::vector<SplitPoint>::const_iterator splitIt;
    for (splitIt = splitlist.begin(); splitIt < splitlist.end(); ++splitIt) {
        split = *splitIt;
        if (splitIt != splitlist.begin()) {
            /* do previous splitpoint */
            insertIntoSegment(segmentBase, prevstart, split.offset - 1, prevtime, split.duration);
        }
        prevstart = split.offset;
        prevtime = split.time;
    }

    if (splitlist.size() == 1) {
        insertIntoSegment(segmentBase, prevstart, 0, prevtime, split.duration);
    } else if (splitlist.size() > 1) {
        insertIntoSegment(segmentBase, prevstart, split.offset - 1, prevtime, split.duration);
    }
}

DashUrl SegmentInformation::getUrlSegment() const
{
    if (baseUrl && baseUrl->hasScheme()) {
        return *(baseUrl);
    } else {
        DashUrl ret = getParentUrlSegment();
        if (baseUrl) {
            ret.append(*(baseUrl));
        }
        return ret;
    }
}

playList *SegmentInformation::getPlayList() const
{
    if (mParent) {
        return mParent->getPlayList();
    } else {
        return nullptr;
    }
}