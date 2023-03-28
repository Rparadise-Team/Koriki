#!/bin/sh
progdir=`dirname "$0"`
emupath=`echo "$0" | awk '{st = index($0,"/../../"); print substr($0,0,st-1)}'`
ext=`echo "$(basename "$1")" | awk -F. '{print tolower($NF)}'`

if [ "$ext" = "miyoocmd" ]; then
    launch="$1"
else
    launch="$emupath/launch.sh"
    emupath=""
fi

chmod a+x "$launch"
"$launch" "$1" "$emupath"
