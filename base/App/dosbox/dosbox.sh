#!/bin/sh

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

setbrightness () {
  bright=$(/customer/app/jsonval brightness)
  brightness=$(($bright*10))
  echo $brightness > /sys/class/pwm/pwmchip0/pwm0/duty_cycle
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
		setbrightness &
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

audiofix_on() {
runsvr=`/customer/app/jsonval audiofix`
if [ "$runsvr" != "1" ] ; then
	echo "Enabled audiofix"
	touch /tmp/audioserver_on
	/mnt/SDCARD/Koriki/bin/audioserver &
	if [ -f /mnt/SDCARD/Koriki/lib/libpadsp.so ]; then
		export LD_PRELOAD=/mnt/SDCARD/Koriki/lib/libpadsp.so
	fi
fi
}

audiofix_off() {
runsvr=`/customer/app/jsonval audiofix`
if [ "$runsvr" != "1" ] ; then
	FILE=/customer/app/axp_test
	echo "Disabled audiofix"
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
}

HOME=/mnt/SDCARD/App/dosbox

cd $HOME
python ./mapper.py
export LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH

volume=$(getvolume)

setvolume &
set_snd_level "${volume}" &

#audiofix_on
sleep 1

./dosbox
sync

#audiofix_off
sleep 1

sync
