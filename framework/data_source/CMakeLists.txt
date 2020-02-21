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


set(SOURCE_FILES ${SOURCE_FILES}
        #data_source_service.cpp
        DataSourceUtils.h
        DataSourceUtils.cpp
        IDataSource.h
        IDataSource.cpp
        #        file_data_source.cpp
        ffmpeg_data_source.cpp
        ffmpeg_data_source.h
        #        alivc_data_source.cpp
        #        alivc_data_source.h
        curl/curl_data_source.cpp
        curl/curl_data_source.h
        curl/CURLShareInstance.cpp
        curl/CURLShareInstance.h
        dataSourceIO.cpp
        dataSourceIO.h
        cache/slice.cpp
        cache/slice.h
        cache/cachedSource.cpp
        cachedSource.h
        cache/ISliceManager.h
        cache/ISliceManager.cpp
        cache/memPool.cpp
        cache/memPool.h
        cache/memPoolSlice.cpp
        cache/memPoolSlice.h
        cache/sliceBufferSource.cpp
        cache/sliceBufferSource.h
        SourceReader.h
        SourceReader.cpp
        proxyDataSource.cpp
        proxyDataSource.h
        dataSourcePrototype.cpp
        dataSourcePrototype.h curl/curlShare.cpp curl/curlShare.h curl/CURLConnection.cpp curl/CURLConnection.h)


include_directories(
#${CICADA_FRAMEWORK_DIR}/inc
 #       ${CICADA_FRAMEWORK_INSTALL_BOOST_INC_DIR}
 #       ${CICADA_FRAMEWORK_DIR}/src/mdf
 #       ${CICADA_FRAMEWORK_DIR}/src
 #      ${CICADA_FRAMEWORK_DIR}/src/message
        ${FFMPEG_SOURCE_DIR}
        ${COMMON_INC_DIR}
)

link_directories(
#${CICADA_FRAMEWORK_INSTALL_LIB_DIR}
        ${CICADA_FRAMEWORK_INSTALL_BOOST_LIB_DIR}
        ${COMMON_LIB_DIR})

if (${TARGET_PLATFORM} STREQUAL "windows")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-inconsistent-missing-override -frtti")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -U_FORTIFY_SOURCE -fstack-protector-all")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -U_FORTIFY_SOURCE -fstack-protector-all")
endif ()

link_libraries(${LINNK_LIBRARYS})

add_library(data_source ${TARGET_LIBRARY_TYPE} ${SOURCE_FILES})
if (HAVE_COVERAGE_CONFIG)
    target_link_libraries(data_source PUBLIC coverage_config)
endif ()
