
set(FFMPEG_INSTALL_DIR_ANDROID ${CMAKE_CURRENT_LIST_DIR}/../external/install/ffmpeg/Android)
set(EXTERN_INSTALL_DIR_ANDROID ${CMAKE_CURRENT_LIST_DIR}/../external/install)

message("FFMPEG_INSTALL_DIR_ANDROID is ${FFMPEG_INSTALL_DIR_ANDROID}")
message("EXTERN_INSTALL_DIR_ANDROID is ${EXTERN_INSTALL_DIR_ANDROID}")
set(COMMON_LIB_DIR
        ${FFMPEG_INSTALL_DIR_ANDROID}/${ANDROID_ABI}/)
set(FFMPEG_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../external/external/ffmpeg/)
set(COMMON_INC_DIR ${COMMON_INC_DIR}
        ${EXTERN_INSTALL_DIR_ANDROID}/ffmpeg/Android/${ANDROID_ABI}/include/
        ${EXTERN_INSTALL_DIR_ANDROID}/../build/ffmpeg/Android/${ANDROID_ABI}/
        ${EXTERN_INSTALL_DIR_ANDROID}/curl/Android/${ANDROID_ABI}/include/
        ${EXTERN_INSTALL_DIR_ANDROID}/openssl/Android/${ANDROID_ABI}/include/
        ${CMAKE_CURRENT_LIST_DIR}/../external/boost/
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/../
        ${FFMPEG_SOURCE_DIR}
        )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_CONSTANT_MACROS")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-#warnings")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations -Wno-#warnings")


if (USEASAN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer -fsanitize-address-use-after-scope")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -fsanitize-address-use-after-scope")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
endif (USEASAN)

set(TARGET_LIBRARY_TYPE STATIC)


set(ENABLE_GLRENDER ON)
