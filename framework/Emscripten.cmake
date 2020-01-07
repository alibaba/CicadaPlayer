

set(COMMON_LIB_DIR ${COMMON_LIB_DIR}
        ${TOPDIR}/external/WasmVideoPlayer/dist/lib
        )
set(COMMON_INC_DIR ${COMMON_INC_DIR}
        ${TOPDIR}/external/WasmVideoPlayer/dist/include
        ${TOPDIR}/external/boost/
        ${PROJECT_SOURCE_DIR})


set(FFMPEG_SOURCE_DIR ${TOPDIR}/external/ffmpeg/)

set(TARGET_LIBRARY_TYPE STATIC)
set(USE_PTHREADS 0)