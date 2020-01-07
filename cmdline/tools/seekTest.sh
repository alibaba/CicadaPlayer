#!/usr/bin/env bash
TOOLS_DIR=$(cd $(dirname "${BASH_SOURCE[0]}"); pwd)
PATH=$PATH:${TOOLS_DIR}
source command.sh
function seekTest() {
    local count=0
    while [[ ${count} -lt $2 ]]
    do
        seekForwardPlayer
        sleep $1s
        let count+=1
    done
}

function seekPreview() {
    playPlayer
    seekTest $1 $2
    playPlayer
}

function speedUpTest() {
    local count=0
    while [[ ${count} -lt $3 ]]
    do
        if [[ "$1" == "up" ]];then
            speedUpPlayer
        else
            speedDownPlayer
        fi
        sleep ${2}s
        let count+=1
    done
}

openPlayer $1 8090
seekTest 2 10;

seekPreview 2 10

speedUpTest down 2 10

seekTest 2 10;

speedUpTest up 2 10

seekTest 2 10;

#quitePlayer
closePlayer