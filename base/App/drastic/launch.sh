#!/bin/sh
mydir=`dirname "$0"`
curvol=$(cat /proc/mi_modules/mi_ao/mi_ao0 | awk '/LineOut/ {if (!printed) {print $8; printed=1}}' | sed 's/,//')

cd $mydir
if [ ! -f "/tmp/.show_hotkeys" ]; then
    touch /tmp/.show_hotkeys
    LD_LIBRARY_PATH=./libs:/customer/lib:/config/lib ./show_hotkeys
fi

export HOME=$mydir
export PATH=$mydir:$PATH
export LD_LIBRARY_PATH=$mydir/libs:$LD_LIBRARY_PATH
export SDL_VIDEODRIVER=mmiyoo
export SDL_AUDIODRIVER=mmiyoo
export EGL_VIDEODRIVER=mmiyoo

CUST_LOGO=1
CUST_CPUCLOCK=1

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

if [ "$CUST_CPUCLOCK" == "1" ]; then
    echo "set customized cpuspeed"
    /mnt/SDCARD/Koriki/bin/cpuclock 1500
fi

set_snd_level() {
    for i in $(seq 1 300); do
        if [ -e /proc/mi_modules/mi_ao/mi_ao0 ]; then
            echo "set_ao_volume 0 ${curvol}dB" > /proc/mi_modules/mi_ao/mi_ao0
            echo "set_ao_volume 1 ${curvol}dB" > /proc/mi_modules/mi_ao/mi_ao0
            echo "Volume set to ${curvol}dB"
            break
        fi
        sleep 0.1
    done
}

set_snd_level &

./drastic "$1"
sync
sync
sync

if [ "$CUST_CPUCLOCK" == "1" ]; then
    echo "set customized cpuspeed"
    /mnt/SDCARD/Koriki/bin/cpuclock 1200
fi

echo $sv > /proc/sys/vm/swappiness
echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
