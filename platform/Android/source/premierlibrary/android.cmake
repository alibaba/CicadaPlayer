
message("FFMPEG_INSTALL_DIR_ANDROID is ${FFMPEG_INSTALL_DIR_ANDROID}")
set(COMMON_LIB_DIR  ${FFMPEG_INSTALL_DIR_ANDROID}/${ANDROID_ABI}/
					${TOPDIR}/apsaraPlayer/install/curl/${ANDROID_ABI}/lib/
					${TOPDIR}/apsaraPlayer/install/openssl/${ANDROID_ABI}/lib/)

set(COMMON_INC_DIR  ${TOPDIR}/framework/)


#native库的位置
set(APSARAPLAYER_NATIVE_SRC_DIR ${TOPDIR}/apsaraPlayer/sources/native/src)

set(TARGET_LIBRARY_TYPE SHARED)
