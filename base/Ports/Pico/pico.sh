#!/bin/sh
export picodir=/mnt/SDCARD/App/pico
export sysdir=/mnt/SDCARD/Koriki
export HOME="$picodir"

runsvr=`/customer/app/jsonval audiofix`

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

    start_time=$(/bin/date +%s)
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

MMP=/customer/app/axp_test

if [ -f "$MMP" ]; then

if [ "$runsvr" != "0" ] ; then
	FILE=/customer/app/axp_test
	
	if [ -f /mnt/SDCARD/Koriki/lib/libpadsp.so ]; then
		unset LD_PRELOAD
	fi

    if [ -f "$FILE" ]; then
        killall audioserver
		killall audioserver.plu
		FILE2=/tmp/audioserver_on
		if [ -f "$FILE2" ]; then
			rm /tmp/audioserver_on
			/mnt/SDCARD/Koriki/bin/freemma
		fi
    else
        killall audioserver
		killall audioserver.min
		FILE2=/tmp/audioserver_on
		if [ -f "$FILE2" ]; then
			rm /tmp/audioserver_on
			/mnt/SDCARD/Koriki/bin/freemma
		fi
    fi
fi

export PATH="$HOME"/bin:$PATH
export LD_LIBRARY_PATH="$HOME"/lib:$LD_LIBRARY_PATH
export SDL_VIDEODRIVER=mmiyoo
export SDL_AUDIODRIVER=mmiyoo
export EGL_VIDEODRIVER=mmiyoo

cd "$picodir"

volume=$(getvolume)
setvolume &
set_snd_level "${volume}" &

echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
mount --bind /mnt/SDCARD/Roms/PICO /mnt/SDCARD/App/pico/.lexaloffle/pico-8/carts

pico8_dyn -splore

umount /mnt/SDCARD/App/pico/.lexaloffle/pico-8/carts
mv /mnt/SDCARD/App/pico/.lexaloffle/pico-8/bbs/carts/*.p8.png /mnt/SDCARD/Roms/PICO/
sync

echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

runsvr=`/customer/app/jsonval audiofix`
if [ "$runsvr" != "0" ] ; then
	touch /tmp/audioserver_on
	/mnt/SDCARD/Koriki/bin/audioserver &
	if [ -f /mnt/SDCARD/Koriki/lib/libpadsp.so ]; then
		export LD_PRELOAD=/mnt/SDCARD/Koriki/lib/libpadsp.so
	fi
else #fixed slow music menu
	/mnt/SDCARD/Koriki/bin/audioserver &
	if [ -f /mnt/SDCARD/Koriki/lib/libpadsp.so ]; then
		export LD_PRELOAD=/mnt/SDCARD/Koriki/lib/libpadsp.so
	fi
	
	sleep 1
	
	FILE=/customer/app/axp_test
	
	if [ -f /mnt/SDCARD/Koriki/lib/libpadsp.so ]; then
		unset LD_PRELOAD
	fi

    if [ -f "$FILE" ]; then
        killall audioserver
		killall -g audioserver.plu
    else
        killall audioserver
		killall -g audioserver.min
    fi
	
	sleep 1
fi

sync
exit

else

show "/mnt/SDCARD/Koriki/images/warning.png"
sleep 10

fi

