#!/bin/sh
echo $0 $*
cd $(dirname "$0")
echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
sync

setvolume () {
  vol=$(/customer/app/jsonval vol)
  volume=$((($vol*3)+40))
  /customer/app/tinymix set 6 $volume
}

getvolume() {
  vol=$(/customer/app/jsonval vol)
  volume=$((($vol*3)-60))
  echo $volume
}

set_snd_level() {
    local target_vol="$1"
    local current_vol
    local start_time
    local elapsed_time

    start_time=$(date +%s)
    while [ ! -e /proc/mi_modules/mi_ao/mi_ao0 ]; do
        sleep 0.2
        elapsed_time=$(( $(date +%s) - start_time ))
        if [ "$elapsed_time" -ge 30 ]; then
            echo "Timed out waiting for /proc/mi_modules/mi_ao/mi_ao0"
            return 1
        fi
    done

    start_time=$(date +%s)
    while true; do
        echo "set_ao_volume 0 ${target_vol}" > /proc/mi_modules/mi_ao/mi_ao0
        echo "set_ao_volume 1 ${target_vol}" > /proc/mi_modules/mi_ao/mi_ao0
        current_vol=$(getvolume)

        if [ "$current_vol" = "$target_vol" ]; then
            echo "Volume set to ${current_vol}dB"
            return 0
        fi

        elapsed_time=$(( $(date +%s) - start_time ))
        if [ "$elapsed_time" -ge 30 ]; then
            echo "Timed out trying to set volume"
            return 1
        fi

        sleep 0.2
    done
}

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

volume=$(getvolume)
setvolume &
set_snd_level "${volume}" &

HOME=/mnt/SDCARD/App/Gmu
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./lib
export LD_PRELOAD=./lib/libSDL-1.2.so.0
SDL_NOMOUSE=1 ./gmu.bin -c gmu.miyoo.conf &> log.txt
sync
unset LD_PRELOAD
echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
sync

get_screen_resolution
