#!/bin/sh
echo $0 $*
progdir=`cd -- "$(dirname "$0")" >/dev/null 2>&1; pwd -P`
ext=`echo "$(basename "$1")" | awk -F. '{print tolower($NF)}'`

cd /mnt/SDCARD/Koriki

if [ "$ext" = "miyoocmd" ]; then
    filename=`basename "$1" .miyoocmd`

    if [ "$filename" = "Enter search term..." ]; then
        mode="search"
    else
        mode="noop"
    fi
elif [ "$1" = "" ]; then
    mode="search"
else
    mode="$1"
fi

echo "launch mode:" $mode

if [ "$mode" = "noop" ]; then
    echo noop >> $progdir/debug.log
elif [ "${mode:0:8}" = "setstate" ]; then
    label="${mode:9}"
    ./bin/search setstate "$label"
elif [ "$mode" = "clear" ]; then
    ./bin/search clear
elif [ "$mode" = "search" ]; then
    ./bin/search
else
    launch=`echo "$1" | awk '{split($0,a,":"); print a[1]}'`
    romfile=`echo "$1" | awk '{split($0,a,":"); print a[2]}'`
    cd /mnt/SDCARD/RetroArch
    chmod a+x "$launch"
    "$launch" "$romfile"
fi