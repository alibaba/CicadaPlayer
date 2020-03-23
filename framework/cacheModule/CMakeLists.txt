cmake_minimum_required(VERSION 3.6)
project(cacheModule)


set(CMAKE_CXX_STANDARD 11)

if (MSVC)
    set(COMMON_INC_DIR ${COMMON_INC_DIR}
        ${TOPDIR}/prebuild/windows/dirent)
endif ()

include_directories(${COMMON_INC_DIR}
        ./
        ../../mediaPlayer/
        ../../framework)

link_directories(${COMMON_LIB_DIR})

add_library(cacheModule STATIC
       
        CacheModule.cpp
        CacheModule.h
        CacheManager.cpp
        CacheManager.h

        cache/CacheConfig.h
        cache/CacheFileRemuxer.cpp
        cache/CacheFileRemuxer.h
        cache/CacheChecker.cpp
        cache/CacheChecker.h
        cache/CachePath.cpp
        cache/CachePath.h
        cache/CacheRet.h
        )

if (HAVE_COVERAGE_CONFIG)
    target_link_libraries(cacheModule PUBLIC coverage_config)
endif ()
