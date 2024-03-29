#!/bin/sh
mydir=`dirname "$0"`
if dmesg|fgrep -q "FB_WIDTH=752"; then
USE_752x560_RES=1
else
USE_752x560_RES=0
fi

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

cd $mydir
if [ ! -f "/tmp/.show_hotkeys" ]; then
    touch /tmp/.show_hotkeys
    LD_LIBRARY_PATH=./libs:/customer/lib:/config/lib:/mnt/SDCARD/Koriki/lib ./show_hotkeys
fi

export HOME=$mydir
export PATH=$mydir:$PATH
export LD_LIBRARY_PATH=$mydir/libs:$LD_LIBRARY_PATH
export SDL_VIDEODRIVER=mmiyoo
export SDL_AUDIODRIVER=mmiyoo
export EGL_VIDEODRIVER=mmiyoo

CUST_LOGO=1
CUST_CPUCLOCK=1

if [ "$USE_752x560_RES" == "1" ]; then
    fbset -g 752 560 752 1120 32
    fbset > fbset.log
fi

cd $mydir
if [ "$CUST_LOGO" == "1" ]; then
    echo "convert resources/logo/0.png to drastic_logo_0.raw"
    echo "convert resources/logo/1.png to drastic_logo_1.raw"
    ./png2raw
fi

sv=`cat /proc/sys/vm/swappiness`

# 60 by default
echo 10 > /proc/sys/vm/swappiness

cd $mydir

volume=$(getvolume)

MAXCPU=$(grep -o '"maxcpu":[0-9]*' ./resources/settings.json | awk -F':' '{print $2}')

if [ "$CUST_CPUCLOCK" == "1" ]; then
    echo "set customized cpuspeed"
    /mnt/SDCARD/Koriki/bin/cpuclock $MAXCPU
fi

setvolume &
set_snd_level "${volume}" &

./drastic "$1"
sync
sync
sync

if [ "$CUST_CPUCLOCK" == "1" ]; then
    echo "set customized cpuspeed"
    /mnt/SDCARD/Koriki/bin/cpuclock 1200
fi

if [ "$USE_752x560_RES" == "1" ]; then
    fbset -g 640 480 640 960 32
fi

echo $sv > /proc/sys/vm/swappiness
echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
