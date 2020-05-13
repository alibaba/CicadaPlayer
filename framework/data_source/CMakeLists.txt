cmake_minimum_required(VERSION 3.6)
project(framework_DataSource)
#if (${TARGET_PLATFORM} STREQUAL "windows")
#else ()
#    add_subdirectory(${TOPDIR}/alitun-api alitun-api.out)
#    set(SOURCE_FILES
#            alitun_data_source.cpp
#            alitun_data_source.h)
#endif ()

set(CMAKE_CXX_STANDARD 11)
add_library(data_source ${TARGET_LIBRARY_TYPE} "")

target_sources(data_source PRIVATE
        DataSourceUtils.h
        DataSourceUtils.cpp
        IDataSource.h
        IDataSource.cpp
        ffmpeg_data_source.cpp
        ffmpeg_data_source.h
        dataSourceIO.cpp
        dataSourceIO.h
        SourceReader.h
        SourceReader.cpp
        proxyDataSource.cpp
        proxyDataSource.h
        dataSourcePrototype.cpp
        dataSourcePrototype.h)

if (ENABLE_CURL_SOURCE)
    target_compile_definitions(data_source PRIVATE ENABLE_CURL_SOURCE)
    target_sources(data_source PRIVATE
            curl/curl_data_source.cpp
            curl/curl_data_source.h
            curl/CURLShareInstance.cpp
            curl/CURLShareInstance.h
            curl/curlShare.cpp
            curl/curlShare.h
            curl/CURLConnection.cpp
            curl/CURLConnection.h
            )
endif ()

if (ENABLE_CACHED_SOURCE)
    target_sources(data_source PRIVATE
            cachedSource.h
            cache/slice.cpp
            cache/slice.h
            cache/cachedSource.cpp
            cache/ISliceManager.h
            cache/ISliceManager.cpp
            cache/memPool.cpp
            cache/memPool.h
            cache/memPoolSlice.cpp
            cache/memPoolSlice.h
            cache/sliceBufferSource.cpp
            cache/sliceBufferSource.h
            )
endif ()


target_include_directories(data_source PRIVATE
        ${FFMPEG_SOURCE_DIR}
        ${COMMON_INC_DIR}
)

link_directories(
        ${CICADA_FRAMEWORK_INSTALL_BOOST_LIB_DIR}
        ${COMMON_LIB_DIR})

if (${TARGET_PLATFORM} STREQUAL "windows")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-inconsistent-missing-override")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -U_FORTIFY_SOURCE -fstack-protector-all")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -U_FORTIFY_SOURCE -fstack-protector-all")
endif ()

target_link_libraries(data_source PRIVATE ${LINNK_LIBRARYS})
if (HAVE_COVERAGE_CONFIG)
    target_link_libraries(data_source PUBLIC coverage_config)
endif ()
