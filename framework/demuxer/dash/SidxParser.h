//
// Created by yuyuan on 2021/05/26.
//

#ifndef DEMUXER_DASH_SIDX_PARSER_H
#define DEMUXER_DASH_SIDX_PARSER_H

#include <stdint.h>

namespace Cicada {
    namespace Dash {

        typedef struct SidxBoxItem_t {
            uint32_t referenced_size;
            uint32_t subsegment_duration;
            uint8_t reference_type;
            uint8_t starts_with_SAP;
            uint8_t SAP_type;
            uint32_t SAP_delta_time;

        } SidxBoxItem;

        typedef struct SidxBox_t {
            uint8_t version;
            uint32_t flags;

            uint32_t reference_ID;
            uint32_t timescale;
            uint64_t earliest_presentation_time;
            uint64_t first_offset;
            uint16_t reference_count = 0;

            SidxBoxItem *items;

        } SidxBox;

        class SidxParser {
        public:
            SidxParser();
            ~SidxParser();
            void ParseSidx(uint8_t *buffer, int64_t size);
            const SidxBox &GetSidxBox();

        private:
            SidxBox mSidxBox;
        };


    }// namespace Dash
}// namespace Cicada


#endif//DEMUXER_DASH_SIDX_PARSER_H
