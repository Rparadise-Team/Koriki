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
