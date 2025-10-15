#!/bin/sh
progdir=`dirname "$0"`
cd $progdir
HOME=/mnt/SDCARD

get_screen_resolution() {
    max_attempts=10
    attempt=0

    echo "get_screen_resolution: start"
    while [ "$attempt" -lt "$max_attempts" ]; do
        screen_resolution=$(cat /tmp/screen_resolution)
        if [ -n "$screen_resolution" ]; then
            echo "get_screen_resolution: success, resolution: $screen_resolution"
            break
        fi
        echo "get_screen_resolution: attempt $attempt failed"
        attempt=$((attempt + 1))
        sleep 0.5
    done

    res_x=$(echo "$screen_resolution" | cut -d 'x' -f 1)
    res_y=$(echo "$screen_resolution" | cut -d 'x' -f 2)
	
	echo "change resolution to $res_x x $res_y"

    fbset -g "$res_x" "$res_y" "$res_x" "$((res_y * 2))" 32
}

fbset -g 640 480 640 960 32

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./lib
export LD_PRELOAD=./lib/libSDL-1.2.so.0

./st

unset LD_PRELOAD
get_screen_resolution