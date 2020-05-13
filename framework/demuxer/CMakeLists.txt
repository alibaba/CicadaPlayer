cmake_minimum_required(VERSION 3.6)
project(framework_Demuxer)

#if (ANDROID_PLATFORM)
#    set(TARGET_PLATFORM Android)
#endif ()

#if (TARGET_PLATFORM STREQUAL "Android")
#    include(../${TARGET_PLATFORM}.cmake)
#endif ()

set(CMAKE_CXX_STANDARD 11)


add_library(demuxer ${TARGET_LIBRARY_TYPE} "")

target_sources(demuxer PRIVATE
        demuxer_service.cpp
        demuxer_service.h
        DemuxerMeta.h
        IDemuxer.h
        IDemuxer.cpp
        avFormatDemuxer.cpp
        avFormatDemuxer.h
        AVBSF.cpp
        AVBSF.h
        sample_decrypt/HLSSampleAesDecrypter.h
        sample_decrypt/HLSSampleAesDecrypter.cpp
        demuxerPrototype.cpp
        demuxerPrototype.h
        avFormatSubtitleDemuxer.cpp
        avFormatSubtitleDemuxer.h
        sample_decrypt/ISampleDecryptor.cpp
        sample_decrypt/ISampleDecryptor.h
        sample_decrypt/SampleDecryptDemuxer.cpp
        sample_decrypt/SampleDecryptDemuxer.h
        sample_decrypt/sampleDecryptDec.c
        sample_decrypt/ISampleDecrypt2c.cpp
        sample_decrypt/ISampleDecrypt2c.h
        decrypto/IAESDecrypt.h
        decrypto/avAESDecrypt.cpp
        decrypto/avAESDecrypt.h
        )

if (ENABLE_HLS_DEMUXER)
    target_compile_definitions(demuxer PRIVATE ENABLE_HLS_DEMUXER)
    target_sources(demuxer PRIVATE
            play_list/segment.cpp
            play_list/Representation.cpp
            play_list/AdaptationSet.cpp
            play_list/Period.cpp
            play_list/playList.h
            play_list/playList.cpp
            play_list/playList_demuxer.h
            play_list/playList_demuxer.cpp
            play_list/playListParser.h
            play_list/playListParser.cpp
            play_list/HlsParser.h
            play_list/HlsParser.cpp
            play_list/HlsTags.cpp
            play_list/Helper.cpp
            play_list/SegmentList.h
            play_list/SegmentList.cpp
            play_list/PlaylistManager.h
            play_list/PlaylistManager.cpp
            play_list/HLSManager.h
            play_list/HLSManager.cpp
            play_list/AbstractStream.h
            play_list/AbstractStream.cpp
            play_list/HLSStream.h
            play_list/HLSStream.cpp
            play_list/SegmentTracker.h
            play_list/SegmentTracker.cpp
            play_list/segment_decrypt/SegmentEncryption.cpp
            play_list/segment_decrypt/ISegDecrypter.cpp
            play_list/segment_decrypt/SegDecryptorFactory.cpp
            play_list/segment_decrypt/AES_128Decrypter.cpp
            play_list/segment_decrypt/ISegDecryptorPrototype.cpp
            play_list/segment_decrypt/ISegDecryptorPrototype.h)
endif ()

if (USE_OPENSSL)
    target_sources(demuxer PRIVATE
            decrypto/OpenSSAESDecrypt.cpp
            decrypto/OpenSSAESDecrypt.h
            decrypto/OpenSSAESEncrypt.cpp
            decrypto/OpenSSAESEncrypt.h
            )
endif ()

target_include_directories(demuxer PRIVATE
        ${FFMPEG_SOURCE_DIR}
        ${COMMON_INC_DIR})

link_directories(
        ${CICADA_FRAMEWORK_INSTALL_BOOST_LIB_DIR}
        ${COMMON_LIB_DIR})

if (TARGET_PLATFORM STREQUAL "windows")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-inconsistent-missing-override")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -U_FORTIFY_SOURCE -fstack-protector-all")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -U_FORTIFY_SOURCE -fstack-protector-all")
endif ()

link_directories(${LINNK_LIBRARYS})

if (HAVE_COVERAGE_CONFIG)
    target_link_libraries(demuxer PUBLIC coverage_config)
endif ()