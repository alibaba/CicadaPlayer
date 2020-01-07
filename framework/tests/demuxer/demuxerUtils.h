//
// Created by pingkai on 2019/12/31.
//

#ifndef CICADAMEDIA_DEMUXERUTILS_H
#define CICADAMEDIA_DEMUXERUTILS_H

#include <string>

void test_mergeHeader(std::string url, bool merge);

void testFirstSeek(const std::string &url, int64_t time, int64_t abs_error);

#endif //CICADAMEDIA_DEMUXERUTILS_H
