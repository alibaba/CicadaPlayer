
message("TOPDIR is ${TOPDIR}")

set(WINDOWS_INSTALL_DIR ${CMAKE_CURRENT_LIST_DIR}/../external/install)

message("WINDOWS_INSTALL_DIR is ${WINDOWS_INSTALL_DIR}")

if (${CMAKE_SIZEOF_VOID_P} MATCHES 4)
    set(ARCH i686)
else ()
    set(ARCH x86_64)
endif ()


set(COMMON_LIB_DIR ${COMMON_LIB_DIR}
        ${WINDOWS_INSTALL_DIR}/curl/win32/${ARCH}/lib
        ${WINDOWS_INSTALL_DIR}/librtmp/win32/${ARCH}/lib
        ${WINDOWS_INSTALL_DIR}/openssl/win32/${ARCH}/lib
        ${WINDOWS_INSTALL_DIR}/ffmpeg/win32/${ARCH}/lib
        #        ${WINDOWS_INSTALL_DIR}/pthread/win32/${ARCH}/lib
        ${WINDOWS_INSTALL_DIR}/fdk-aac/win32/${ARCH}/lib
        /home/pingkai/Downloads/SDL2-devel-2.0.10-mingw/SDL2-2.0.10/x86_64-w64-mingw32/lib
        )
set(FFMPEG_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../external/external/ffmpeg/)
set(COMMON_INC_DIR ${COMMON_INC_DIR}
        ${WINDOWS_INSTALL_DIR}/curl/win32/${ARCH}/include
        ${WINDOWS_INSTALL_DIR}/librtmp/win32/${ARCH}/include
        ${WINDOWS_INSTALL_DIR}/openssl/win32/${ARCH}/include
        ${WINDOWS_INSTALL_DIR}/ffmpeg/win32/${ARCH}/include
        ${PROJECT_SOURCE_DIR}
        ${FFMPEG_SOURCE_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../external/boost
        /home/pingkai/Downloads/SDL2-devel-2.0.10-mingw/SDL2-2.0.10/x86_64-w64-mingw32/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBUILDING_LIBCURL")
link_libraries(ws2_32)

set(TARGET_LIBRARY_TYPE STATIC)

set(ENABLE_GLRENDER OFF)
set(ENABLE_SDL ON)
set(BUILD_TEST OFF)
add_definitions(-D __STDC_FORMAT_MACROS)
