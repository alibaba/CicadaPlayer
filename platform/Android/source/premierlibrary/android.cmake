
#组件的头文件
#set(FRAMEWORK_INC_DIR ${TOPDIR}/alivc_framework/src
#                      ${TOPDIR}/alivc_framework/inc)

#静态库的位置
#set(FRAMEWORK_LIB_DIR   ${TOPDIR}/alivc_framework/bin/framework_player/lib/android/${ANDROID_ABI}/
#                        ${TOPDIR}/alivc_framework/bin/framework_player/lib/android/${ANDROID_ABI}/lib/)
message("FFMPEG_INSTALL_DIR_ANDROID is ${FFMPEG_INSTALL_DIR_ANDROID}")
set(COMMON_LIB_DIR  ${FFMPEG_INSTALL_DIR_ANDROID}/${ANDROID_ABI}/
					${TOPDIR}/apsaraPlayer/install/curl/${ANDROID_ABI}/lib/
					${TOPDIR}/apsaraPlayer/install/openssl/${ANDROID_ABI}/lib/)

	#使用alitun里面的				${TOPDIR}/apsaraPlayer/install/librtmp/${ANDROID_ABI}/lib/
set(COMMON_INC_DIR  ${TOPDIR}/framework/)

#rtxp的库位置
set(ALITUN_INC_DIR ${TOPDIR}/alitun-api/output/Android/${ANDROID_ABI}/include/ )
set(ALITUN_LIB_DIR ${TOPDIR}/alitun-api/output/Android/${ANDROID_ABI}/lib/)

set(CONAN_INC_DIR ${TOPDIR}/prebuild/Android/Conan/alivc_conan_lib/inc/ )
set(CONAN_LIB_DIR ${TOPDIR}/prebuild/Android/Conan/alivc_conan_lib/${ANDROID_ABI}/)

#native库的位置
set(APSARAPLAYER_NATIVE_SRC_DIR ${TOPDIR}/apsaraPlayer/sources/native/src)

set(TARGET_LIBRARY_TYPE SHARED)
