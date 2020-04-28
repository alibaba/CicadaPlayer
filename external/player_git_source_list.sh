#!/usr/bin/env bash

function clone_git() {

if [[ -n "$3" ]];then
    git clone $1 -b $2 $3
  else
    git clone $1 -b $2
  fi

}

if [[ -z "${FFMPEG_GIT}" ]];then
    FFMPEG_GIT=https://github.com/FFmpeg/FFmpeg.git
fi
if [[ -z "${FFMPEG_BRANCH}" ]];then
    FFMPEG_BRANCH=n4.2.1
fi

if [[ -z "${FFMPEG_NEED_PATCH}" ]];then
    FFMPEG_NEED_PATCH=TRUE
fi
clone_git "$FFMPEG_GIT" "$FFMPEG_BRANCH" ffmpeg




if [ -z "${OPENSSL_GIT}" ];then
    OPENSSL_GIT="https://github.com/openssl/openssl.git"
fi
OPENSSL_BRANCH="OpenSSL_1_1_1g"
clone_git $OPENSSL_GIT $OPENSSL_BRANCH

if [ -z "${CURL_GIT}" ];then
    CURL_GIT="https://github.com/curl/curl.git"
fi
CURL_BRANCH="curl-7_63_0"
clone_git $CURL_GIT "$CURL_BRANCH"

if [[ -z "${DAV1D_EXTERNAL_DIR}" ]];then
    if [[ -n "${DAV1D_SOURCE_DIR}" ]];then
        echo "check out dav1d"
        if [ -z "${DAV1D_GIT}" ];then
            DAV1D_GIT="https://github.com/videolan/dav1d.git"
        fi
        DAV1D_BRANCH="0.6.0"
        clone_git $DAV1D_GIT $DAV1D_BRANCH
    fi
fi
