#!/usr/bin/env bash
git_v=$(git describe  --always)
time_v=$(date "+%Y%m%d_%H:%M:%S")

out="#define build_version \"${time_v}_${git_v}\""
echo "$out"
