//
// Created by yuyuan on 2021/05/26.
//

#include "SidxParser.h"
#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif
#include "utils/frame_work_log.h"
#include <string>
#include <cstring>

using namespace Cicada::Dash;

SidxParser::SidxParser()
{}

SidxParser::~SidxParser()
{
    if (mSidxBox.reference_count != 0) {
        free(mSidxBox.items);
    }
}

static inline uint32_t Get24bBE(const uint8_t *p)
{
    return ((p[0] << 16) + (p[1] << 8) + p[2]);
}

#ifndef __APPLE__

static inline uint64_t ntohll(uint64_t x)
{
    int a = 1;
    uint8_t *b = (uint8_t *) &a;
    if (*b == 1) {
        // little endian
        return ((x & 0x00000000000000FFULL) << 56) | ((x & 0x000000000000FF00ULL) << 40) | ((x & 0x0000000000FF0000ULL) << 24) |
               ((x & 0x00000000FF000000ULL) << 8) | ((x & 0x000000FF00000000ULL) >> 8) | ((x & 0x0000FF0000000000ULL) >> 24) |
               ((x & 0x00FF000000000000ULL) >> 40) | ((x & 0xFF00000000000000ULL) >> 56);
    } else {
        // big endian
        return x;
    }
}

#endif
static inline uint8_t SidxGet1Byte(uint8_t *buffer, uint64_t &offset, uint64_t size)
{
    if (offset >= size) {
        offset = size - 1;
        return 0;
    }
    uint8_t ret = buffer[offset];
    offset++;
    return ret;
}

static inline uint16_t SidxGet2Bytes(uint8_t *buffer, uint64_t &offset, uint64_t size)
{
    if (offset + 1 >= size) {
        offset = size - 1;
        return 0;
    }
    uint16_t ret;
    memcpy(&ret, buffer + offset, sizeof(ret));
    offset += 2;
    return ntohs(ret);
}

static inline uint32_t SidxGet3Bytes(uint8_t *buffer, uint64_t &offset, uint64_t size)
{
    if (offset + 2 >= size) {
        offset = size - 1;
        return 0;
    }
    uint32_t ret = Get24bBE(buffer + offset);
    offset += 3;
    return ret;
}

static inline uint32_t SidxGet4Bytes(uint8_t *buffer, uint64_t &offset, uint64_t size)
{
    if (offset + 3 >= size) {
        offset = size - 1;
        return 0;
    }
    uint32_t ret;
    memcpy(&ret, buffer + offset, sizeof(ret));
    offset += 4;
    return ntohl(ret);
}

static inline uint64_t SidxGet8Bytes(uint8_t *buffer, uint64_t &offset, uint64_t size)
{
    if (offset + 7 >= size) {
        offset = size - 1;
        return 0;
    }
    uint64_t ret;
    memcpy(&ret, buffer + offset, sizeof(ret));
    offset += 8;
    return ntohll(ret);
}

void SidxParser::ParseSidx(uint8_t *buffer, int64_t size)
{
    uint64_t offset = 0;
    uint32_t boxSize = SidxGet4Bytes(buffer, offset, size);
    uint8_t typeS = SidxGet1Byte(buffer, offset, size);
    uint8_t typeI = SidxGet1Byte(buffer, offset, size);
    uint8_t typeD = SidxGet1Byte(buffer, offset, size);
    uint8_t typeX = SidxGet1Byte(buffer, offset, size);
    if (typeS != 's' || typeI != 'i' || typeD != 'd' || typeX != 'x') {
        return;
    }
    
    if (boxSize == 1) {
        offset += 8;
    }
    mSidxBox.version = SidxGet1Byte(buffer, offset, size);
    mSidxBox.flags = SidxGet3Bytes(buffer, offset, size);
    mSidxBox.reference_ID = SidxGet4Bytes(buffer, offset, size);
    mSidxBox.timescale = SidxGet4Bytes(buffer, offset, size);

    if (mSidxBox.version == 0) {
        mSidxBox.earliest_presentation_time = SidxGet4Bytes(buffer, offset, size);
        mSidxBox.first_offset = SidxGet4Bytes(buffer, offset, size);
    } else {
        mSidxBox.earliest_presentation_time = SidxGet8Bytes(buffer, offset, size);
        mSidxBox.first_offset = SidxGet8Bytes(buffer, offset, size);
    }

    uint16_t reserved, count;
    reserved = SidxGet2Bytes(buffer, offset, size);
    count = SidxGet2Bytes(buffer, offset, size);
    if (count == 0) {
        return;
    }

    mSidxBox.reference_count = count;
    mSidxBox.items = (SidxBoxItem *) malloc(count * sizeof(SidxBoxItem));
    if (mSidxBox.items == NULL) {
        return;
    }
    for (unsigned i = 0; i < count; i++) {
        SidxBoxItem *item = mSidxBox.items + i;
        uint32_t tmp;

        tmp = SidxGet4Bytes(buffer, offset, size);
        item->reference_type = tmp >> 31;
        item->referenced_size = tmp & 0x7fffffff;
        item->subsegment_duration = SidxGet4Bytes(buffer, offset, size);

        tmp = SidxGet4Bytes(buffer, offset, size);
        item->starts_with_SAP = tmp >> 31;
        item->SAP_type = (tmp >> 24) & 0x70;
        item->SAP_delta_time = tmp & 0xfffffff;
    }

    AF_LOGI("parse sidx box, version %d, flags 0x%x, ref_ID %u, timescale %u, ref_count %hu, first subsegmt duration %u", mSidxBox.version,
            mSidxBox.flags, mSidxBox.reference_ID, mSidxBox.timescale, mSidxBox.reference_count, mSidxBox.items[0].subsegment_duration);
}

const SidxBox &SidxParser::GetSidxBox()
{
    return mSidxBox;
}