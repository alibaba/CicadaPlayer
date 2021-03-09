//
// Created by yuyuan on 2021/03/09.
//

#include "SegmentTimeline.h"
#include <algorithm>

using namespace Cicada::Dash;

SegmentTimeline::SegmentTimeline(AttrsNode *parent_) : AttrsNode(Type::Timeline, parent_)
{
    totalLength = 0;
}

SegmentTimeline::~SegmentTimeline()
{
    std::list<Element *>::iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        delete *it;
    }
}

void SegmentTimeline::addElement(uint64_t number, int64_t d, uint64_t r, int64_t t)
{
    Element *element = new (std::nothrow) Element(number, d, r, t);
    if (element) {
        if (!elements.empty() && !t) {
            const Element *el = elements.back();
            element->t = el->t + (el->d * (el->r + 1));
        }
        elements.push_back(element);
        totalLength += (d * (r + 1));
    }
}

int64_t SegmentTimeline::getMinAheadScaledTime(uint64_t number) const
{
    int64_t totalscaledtime = 0;

    if (!elements.size() || minElementNumber() > number || maxElementNumber() < number) {
        return 0;
    }

    std::list<Element *>::const_reverse_iterator it;
    for (it = elements.rbegin(); it != elements.rend(); ++it) {
        const Element *el = *it;
        if (number > el->number + el->r) {
            break;
        } else if (number < el->number) {
            totalscaledtime += (el->d * (el->r + 1));
        } else /* within repeat range */
        {
            totalscaledtime += el->d * (el->number + el->r - number);
        }
    }

    return totalscaledtime;
}

uint64_t SegmentTimeline::getElementNumberByScaledPlaybackTime(int64_t scaled) const
{
    const Element *prevel = nullptr;
    std::list<Element *>::const_iterator it;

    if (!elements.size()) {
        return 0;
    }

    for (it = elements.begin(); it != elements.end(); ++it) {
        const Element *el = *it;
        if (scaled >= el->t) {
            if ((uint64_t) scaled < el->t + (el->d * el->r)) {
                return el->number + (scaled - el->t) / el->d;
            }
        }
        /* might have been discontinuity */
        else {
            if (prevel) /* > prev but < current */
            {
                return prevel->number + prevel->r;
            } else /* << first of the list */
            {
                return el->number;
            }
        }
        prevel = el;
    }

    /* time is >> any of the list */
    return prevel->number + prevel->r;
}

bool SegmentTimeline::getScaledPlaybackTimeDurationBySegmentNumber(uint64_t number, int64_t *time, int64_t *duration) const
{
    std::list<Element *>::const_iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        const Element *el = *it;
        if (number >= el->number) {
            if (number <= el->number + el->r) {
                *time = el->t + el->d * (number - el->number);
                *duration = el->d;
                return true;
            }
        }
    }
    return false;
}

int64_t SegmentTimeline::getScaledPlaybackTimeByElementNumber(uint64_t number) const
{
    int64_t time = 0, duration = 0;
    (void) getScaledPlaybackTimeDurationBySegmentNumber(number, &time, &duration);
    return time;
}

int64_t SegmentTimeline::getTotalLength() const
{
    return totalLength;
}

uint64_t SegmentTimeline::maxElementNumber() const
{
    if (elements.empty()) {
        return 0;
    }

    const Element *e = elements.back();
    return e->number + e->r;
}

uint64_t SegmentTimeline::minElementNumber() const
{
    if (elements.empty()) {
        return 0;
    }
    return elements.front()->number;
}

uint64_t SegmentTimeline::getElementIndexBySequence(uint64_t number) const
{
    std::list<Element *>::const_iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        const Element *el = *it;
        if (number >= el->number) {
            if (number <= el->number + el->r) {
                return std::distance(elements.begin(), it);
            }
        }
    }
    return std::numeric_limits<uint64_t>::max();
}

void SegmentTimeline::pruneByPlaybackTime(int64_t time)
{
    const Timescale timescale = inheritTimescale();
    uint64_t num = getElementNumberByScaledPlaybackTime(timescale.ToScaled(time));
    pruneBySequenceNumber(num);
}

size_t SegmentTimeline::pruneBySequenceNumber(uint64_t number)
{
    size_t prunednow = 0;
    while (elements.size()) {
        Element *el = elements.front();
        if (el->number >= number) {
            break;
        } else if (el->number + el->r >= number) {
            uint64_t count = number - el->number;
            el->number += count;
            el->t += count * el->d;
            el->r -= count;
            prunednow += count;
            totalLength -= count * el->d;
            break;
        } else {
            prunednow += el->r + 1;
            elements.pop_front();
            totalLength -= (el->d * (el->r + 1));
            delete el;
        }
    }

    return prunednow;
}

void SegmentTimeline::updateWith(SegmentTimeline &other)
{
    if (elements.empty()) {
        while (other.elements.size()) {
            elements.push_back(other.elements.front());
            other.elements.pop_front();
        }
        return;
    }

    Element *last = elements.back();
    while (other.elements.size()) {
        Element *el = other.elements.front();
        other.elements.pop_front();

        if (last->contains(el->t)) {// Same element, but prev could have been middle of repeat
            const uint64_t count = (el->t - last->t) / last->d;
            totalLength -= (last->d * (last->r + 1));
            last->r = std::max(last->r, el->r + count);
            totalLength += (last->d * (last->r + 1));
            delete el;
        } else if (el->t < last->t) {
            delete el;
        } else {// Did not exist in previous list
            totalLength += (el->d * (el->r + 1));
            elements.push_back(el);
            el->number = last->number + last->r + 1;
            last = el;
        }
    }
}

SegmentTimeline::Element::Element(uint64_t number_, int64_t d_, uint64_t r_, int64_t t_)
{
    number = number_;
    d = d_;
    t = t_;
    r = r_;
}

bool SegmentTimeline::Element::contains(int64_t time) const
{
    if (time >= t && time < t + (int64_t)(r + 1) * d) {
        return true;
    }
    return false;
}
