cmake_minimum_required(VERSION 3.6)
project(framework)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)

if (IOS)
    message("build ios")
    set(TARGET_PLATFORM iOS)
endif ()

if (ANDROID)
    set(TARGET_PLATFORM Android)
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES Windows)
    message("build windows")
    set(TARGET_PLATFORM windows)
elseif (${CMAKE_SYSTEM_NAME} MATCHES Emscripten)
    message("build Emscripten")
    set(TARGET_PLATFORM Emscripten)
endif ()

if (TARGET_PLATFORM)
else ()
    set(TARGET_PLATFORM macOSX)
endif ()
message("TARGET_PLATFORM is ${TARGET_PLATFORM}")

set(TOPDIR ${PROJECT_SOURCE_DIR}/..)
include(${TARGET_PLATFORM}.cmake)

if (EMSCRIPTEN)
    add_compile_options(-s USE_PTHREADS=${USE_PTHREADS} -Os)
    add_link_options(-s ERROR_ON_MISSING_LIBRARIES=0)
endif ()


if ($ENV{VERSION})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVERSION=$ENV{VERSION}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVERSION=$ENV{VERSION}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVERSION=$ENV{VERSION}")
endif ()

include(module_config.cmake)
add_subdirectory(demuxer)
add_subdirectory(codec)
add_subdirectory(utils)
add_subdirectory(filter)
add_subdirectory(render)
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
else ()
    add_subdirectory(communication)
endif ()
if (ENABLE_CACHE_MODULE)
    set(ENABLE_MUXER ON)
    add_definitions(-DENABLE_CACHE_MODULE)
    add_subdirectory(cacheModule)
endif ()
if (ENABLE_MUXER)
    add_definitions(-DENABLE_MUXER)
    add_subdirectory(muxer)
endif ()

if (NOT EMSCRIPTEN)
    add_subdirectory(data_source)
endif ()
add_subdirectory(WebAssemblyPlayer)

set(CMAKE_CXX_STANDARD 11)

set(DATA_SOURCE_TEST_FILES data_source/data_source_test.cpp
        utils/tty.c)

set(DEMUXER_TEST_FILES demuxer_test.cpp
        utils/tty.c)

set(TIMED_QUEUE_FILES utils/TimedEventQueue/TimedEventQueue.cpp
        utils/TimedEventQueue/TimedQueueTest.cpp)

include_directories(
        ${COMMON_INC_DIR}
        ${FFMPEG_SOURCE_DIR}
)


link_directories(
        ${COMMON_LIB_DIR})

message(xxx $ENV{TOP_DIR})

if (EMSCRIPTEN)
    add_executable(WAPlayer WebAssemblyPlayer/WABitStreamDecoder.cpp)
    target_link_libraries(WAPlayer PRIVATE
            bitStreamDecoder
            demuxer
            videodec
            avformat
            avcodec
            avutil
            ssl
            crypto
            framework_utils
            z
            bz2
            iconv)
    if (EMSCRIPTEN)
        set(EXPORTED_FUNCTIONS "[ \
		'_BSDecoderSetReadCallBack', \
		'_BSDecoderEOSCallBack', \
        '_BSDecoderSetGetBuffer', \
        '_BSDecoderSetInfoCallBackVideo', \
        '_BSDecoderSetInfoCallBackAudio', \
        '_BSDecoderSetFrameCallBack', \
        '_BSDecoderPrepare', \
        '_BSDecoderStart', \
        '_BSDecoderFlush', \
        '_BSDecoderStop', \
        '_BSDecoderSetPollDataCallBack', \
        '_BSDecoderSetErrorCallBack', \
        '_BSDecoderSetPollRawBufferCallback', \
        '_test' \
    ]")
        target_link_options(WAPlayer
                PUBLIC
                "SHELL:-s EXPORTED_FUNCTIONS='${EXPORTED_FUNCTIONS}'
            -s WASM=1
            -s ALLOW_MEMORY_GROWTH=1
            -s TOTAL_MEMORY=67108864
            -s EXTRA_EXPORTED_RUNTIME_METHODS='['addFunction']'
            -s RESERVED_FUNCTION_POINTERS=14
            -s ABORTING_MALLOC=0
    ")
    endif ()

endif ()

if (BUILD_TEST)
    #   add_subdirectory(tests)
    #        link_libraries(
    #                demuxer
    #                videodec
    #                data_source
    #                framework_utils
    #                avformat
    #                avcodec
    #                swresample
    #                avutil
    #                swscale
    #                iconv
    #                z
    #                bz2
    #                curl
    #                ssl
    #                crypto
    #                ${FRAMEWORK_LIBS})
    #        add_executable(DemuxerTest ${DEMUXER_TEST_FILES})
    #       add_executable(DataSourceTest ${DATA_SOURCE_TEST_FILES})
    #    #    #    add_executable(simple_player ${PlAYER_TEST_FILES})
    #    #    add_executable(TimedEventQueue ${TIMED_QUEUE_FILES})
    #    #    add_executable(curl_test curl_test.cpp)
    #    #    add_executable(errorTest utils/errors/error_test.cpp)
    #    #    add_executable(clock_test ${CLOCK_TEST_FILES})
    #    #    add_executable(property_test utils/testProperty.c)
    #    #    add_executable(afTreadTest utils/afThreadTest.cpp)
    #    add_executable(ffmpeg_test ffmpeg/ffmpeg_test.cpp)
    #    target_link_libraries(ffmpeg_test
    #            demuxer
    #            videodec
    #            avformat
    #            avcodec
    #            avutil
    #            framework_utils
    #            z
    #            bz2
    #            ssl
    #            crypto
    #            SDL2
    #            iconv
    #            )
    #
    #    if (ENABLE_SDL)
    #        add_executable(sdl_test ffmpeg/sdl_test.cpp)
    #        target_link_libraries(sdl_test
    #                render
    #                demuxer
    #                videodec
    #                avformat
    #                avcodec
    #                avutil
    #                swresample
    #                avfilter
    #                framework_utils
    #                framework_filter
    #                z
    #                bz2
    #                ssl
    #                crypto
    #                SDL2
    #                iconv
    #                )
    #        add_executable(filter_audio_test filter/avFilterAudio_test.cpp)
    #        target_link_libraries(filter_audio_test
    #                render
    #                bitStreamDecoder
    #                demuxer
    #                videodec
    #                avformat
    #                avcodec
    #                avutil
    #                swresample
    #                avfilter
    #                framework_utils
    #                framework_filter
    #                z
    #                bz2
    #                ssl
    #                crypto
    #                iconv
    #                SDL2
    #                )
    #    endif ()
    #    #   add_executable(vsyncTest render/test_vsync.cpp)
    #    #    target_link_libraries(vsyncTest PRIVATE
    #    #            render
    #    #            framework_utils
    #    #            avformat
    #    #            avcodec
    #    #            avutil
    #    #            ssl
    #    #            crypto
    #    #            bz2
    #    #            z
    #    #            iconv
    #    #            )
    #    if (ENABLE_SDL)
    #        add_executable(audioRender_test render/audioRender_test.cpp)
    #        target_link_libraries(audioRender_test
    #                render
    #                bitStreamDecoder
    #                demuxer
    #                videodec
    #                avformat
    #                avcodec
    #                avutil
    #                swresample
    #                avfilter
    #                framework_utils
    #                framework_filter
    #                z
    #                bz2
    #                ssl
    #                crypto
    #                iconv
    #                SDL2
    #                )
    #    endif ()
    #add_executable(sliceTest data_source/cache/sliceTest.cpp)
    #add_executable(cachedSourceTest data_source/cache/cachedSourceTest.cpp)
    #add_executable(cachedDemuxerTest cachedDemuxerTest.cpp)
    #add_executable(MuxerTest ${MUXER_TEST_FILES})
endif ()
