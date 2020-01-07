#!/usr/bin/env bash
function openPlayer() {
   if [[ $# -lt 2 ]];then
       echo "$0 ip port fd"
       return 1;
   fi
   exec 8>/dev/udp/$1/$2
   if [[ "$?" != "0" ]];then
       echo "open /dev/udp/$1/$2 error"
   fi
   return 0;
}

function closePlayer() {
    exec 8>&-
}

function playPlayer() {
    echo " " >&8
}

function quitePlayer() {
    echo q >&8
}

function speedUpPlayer() {
    echo + >&8
}

function speedDownPlayer() {
    echo - >&8
}

function seekForwardPlayer() {
    echo ">" >&8
}

function seekBackPlayer() {
    echo "<" >&8
}

function volumeUpPlayer() {
    echo "V" >&8
}

function volumeDownPlayer(){
    echo "v" >&8
}