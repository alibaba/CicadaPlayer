//
// Created by pingkai on 2019/12/31.
//

#ifndef CICADAMEDIA_DEMUXERUTILS_H
#define CICADAMEDIA_DEMUXERUTILS_H

#include <string>
#include <framework/utils/AFMediaType.h>
#include <framework/demuxer/IDemuxer.h>

void test_mergeHeader(std::string url, Cicada::header_type merge);

void test_mergeAudioHeader(const std::string& url , Cicada::header_type  merge);

void testFirstSeek(const std::string &url, int64_t time, int64_t abs_error);

void test_encryptionInfo(const std::string& url, Stream_type streamType, Cicada::header_type merge);

void test_metaKeyInfo(const std::string& url, Stream_type streamType);

void test_csd(const std::string& url , Cicada::header_type merge);

#endif //CICADAMEDIA_DEMUXERUTILS_H
