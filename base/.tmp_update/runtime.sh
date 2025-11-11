#!/bin/sh

if dmesg|fgrep -q "FB_WIDTH=640"; then
	export SCREEN_WIDTH=640
	export SCREEN_HEIGHT=480
	export SUBMODEL="MM"
	export screen_resolution="640x480"
fi

if dmesg|fgrep -q "FB_WIDTH=752"; then
	export SCREEN_WIDTH=752
	export SCREEN_HEIGHT=560
	if [ ! -f /customer/app/axp_test ]; then
		export SUBMODEL="MMv4"
	else
		export SUBMODEL="MMFLIP"
	fi
	export screen_resolution="752x560"
fi

export SDCARD_PATH="/mnt/SDCARD"
export HOME="${SDCARD_PATH}"
SETTINGS_INT_FILE="/appconfigs/system.json"
SETTINGS_EXT_FILE="/mnt/SDCARD/system.json"

export SYSTEM_PATH="${SDCARD_PATH}/Koriki"

export LD_LIBRARY_PATH="${SYSTEM_PATH}/lib:${LD_LIBRARY_PATH}"
export PATH="${SYSTEM_PATH}/bin:${PATH}"

export SWAPFILE="/mnt/SDCARD/cachefile"
export CPUSAVE="/mnt/SDCARD/.simplemenu/cpu.sav"
export GOVSAVE="/mnt/SDCARD/.simplemenu/governor.sav"
export SPEEDSAVE="/mnt/SDCARD/.simplemenu/speed.sav"

export RETROARCH_PATH="/mnt/SDCARD/RetroArch"

# Detect flash type
if dmesg|fgrep -q "[FSP] Flash is detected (0x1100, 0x68, 0x40, 0x18) ver1.1"; then
	export SETTINGS_FILE="$SETTINGS_EXT_FILE"
	# Create v3 config file is this is not found in the root of SDCARD
	if [ ! -f "$SETTINGS_FILE" ]; then
		if [ -f "/mnt/SDCARD/system.bak" ]; then
			mv "/mnt/SDCARD/system.bak" "$SETTINGS_FILE"
		else
			cp "${SYSTEM_PATH}"/assets/system.json "$SETTINGS_FILE"
		fi
	fi
	if [ ! -s "$SETTINGS_FILE" ]; then
		if [ -f "/mnt/SDCARD/system.bak" ]; then
			mv "/mnt/SDCARD/system.bak" "$SETTINGS_FILE"
		else
			cp "${SYSTEM_PATH}"/assets/system.json "$SETTINGS_FILE"
		fi
	fi
else
	if [ -f /appconfigs/system.json.old ]; then
		export SETTINGS_FILE="$SETTINGS_EXT_FILE"
		# Create v4 config file is this is not found in the root of SDCARD
		if [ "$SUBMODEL" == "MMv4" ]; then
			if [ ! -f "$SETTINGS_FILE" ]; then
				if [ -f "/mnt/SDCARD/system.bak" ]; then
					mv "/mnt/SDCARD/system.bak" "$SETTINGS_FILE"
				else
					cp "${SYSTEM_PATH}"/assets/system-v4.json "$SETTINGS_FILE"
				fi
			fi
			if [ ! -s "$SETTINGS_FILE" ]; then
				if [ -f "/mnt/SDCARD/system.bak" ]; then
					mv "/mnt/SDCARD/system.bak" "$SETTINGS_FILE"
				else
					cp "${SYSTEM_PATH}"/assets/system-v4.json "$SETTINGS_FILE"
				fi
			fi
		fi
		if [ "$SUBMODEL" == "MM" ]; then
			if [ ! -f "$SETTINGS_FILE" ]; then
				if [ -f "/mnt/SDCARD/system.bak" ]; then
					mv "/mnt/SDCARD/system.bak" "$SETTINGS_FILE"
				else
					cp "${SYSTEM_PATH}"/assets/system.json "$SETTINGS_FILE"
				fi
			fi
			if [ ! -s "$SETTINGS_FILE" ]; then
				if [ -f "/mnt/SDCARD/system.bak" ]; then
					mv "/mnt/SDCARD/system.bak" "$SETTINGS_FILE"
				else
					cp "${SYSTEM_PATH}"/assets/system.json "$SETTINGS_FILE"
				fi
			fi
		fi
	else
		export SETTINGS_FILE="$SETTINGS_INT_FILE"
	fi
fi

# rename root sdcard system.json in mini flip
if [ "$SUBMODEL" == "MMFLIP" ]; then
	if [ -f "$SETTINGS_EXT_FILE" ]; then
		mv "$SETTINGS_EXT_FILE" "/mnt/SDCARD/system.bak"
	fi
else
	if [ -f "/mnt/SDCARD/system.bak" ]; then
		mv "/mnt/SDCARD/system.bak" "$SETTINGS_EXT_FILE"
	fi
fi

# fixed bad system.json in miniflip
if [ "$SUBMODEL" == "MMFLIP" ]; then
	if ! grep -q "24hourclock" "$SETTINGS_FILE"; then
		cp "${SYSTEM_PATH}"/assets/system.mmf.json "$SETTINGS_FILE"
	fi
fi

# Detect model and init charger GPIO
if [ ! -f "/customer/app/axp_test" ]; then
	export MODEL="MM"
	if [ ! -f "/sys/devices/gpiochip0/gpio/gpio59/direction" ]; then
		echo 59 > "/sys/class/gpio/export"
		echo in > "/sys/devices/gpiochip0/gpio/gpio59/direction"
	fi
else
	export MODEL="MMP"
fi

#fix onion broken boot in koriki
if [ ! -f "$SETTINGS_FILE" ]; then
	if [ "$MODEL" == "MM" ]; then
		cp "${SYSTEM_PATH}"/assets/system.json "$SETTINGS_FILE"
	fi
	if [ "$MODEL" == "MMP" ]; then
		cp "${SYSTEM_PATH}"/assets/system.mmp.json "$SETTINGS_FILE"
	fi
	if [ "$MODEL" == "MMFLIP" ]; then
		cp "${SYSTEM_PATH}"/assets/system.mmf.json "$SETTINGS_FILE"
	fi
fi

resize() {
# Verificar si no existe el archivo "resized"
if [ ! -f "$SDCARD_PATH/RESIZED" ]; then
	
	export TMP_PATH="/tmp/fatresize"
	export TMP_LIB="/tmp/fatresize/lib"
	
	echo "Iniciando redimensionamiento de la partición FAT32..."
	
	# Crear directorio temporal para fatresize y sus dependencias
	mkdir -p $TMP_PATH
	mkdir -p $TMP_LIB
	
	# Copiar fatresize y sus bibliotecas necesarias
	echo "Copiando parted y dependencias..."
	cp "$SDCARD_PATH/Koriki/bin/fdisk" $TMP_PATH/
	cp "$SDCARD_PATH/Koriki/bin/fsck.fat" $TMP_PATH/
	cp "$SDCARD_PATH/Koriki/bin/fatresize" $TMP_PATH/
	cp "$SDCARD_PATH/Koriki/bin/parted" $TMP_PATH/
	cp "$SDCARD_PATH/Koriki/bin/partprobe" $TMP_PATH/
	cp "$SDCARD_PATH/Koriki/bin/show" $TMP_PATH/
	cp "$SDCARD_PATH/Koriki/images/resize.png" $TMP_PATH/
	chmod +x $TMP_PATH/fdisk
	chmod +x $TMP_PATH/fsck.fat
	chmod +x $TMP_PATH/fatresize
	chmod +x $TMP_PATH/parted
	chmod +x $TMP_PATH/partprobe
	chmod +x $TMP_PATH/show
	cp "$SDCARD_PATH/Koriki/lib/libfdisk.so.1" $TMP_LIB/
	cp "$SDCARD_PATH/Koriki/lib/libsmartcols.so.1" $TMP_LIB/
	cp "$SDCARD_PATH/Koriki/lib/libparted-fs-resize.so.0" $TMP_LIB/
	cp "$SDCARD_PATH/Koriki/lib/libparted.so.2" $TMP_LIB/
	cp "$SDCARD_PATH/Koriki/lib/libblkid.so.1" $TMP_LIB/
	cp "$SDCARD_PATH/Koriki/lib/libuuid.so.1" $TMP_LIB/
	cp "$SDCARD_PATH/Koriki/lib/libpng16.so.16" $TMP_LIB/
	cp "$SDCARD_PATH/Koriki/lib/libncurses.so.6" $TMP_LIB/
	cp "$SDCARD_PATH/Koriki/lib/libmsettings.so" $TMP_LIB/
	chmod +x $TMP_LIB/libfdisk.so.1
	chmod +x $TMP_LIB/libsmartcols.so.1
	chmod +x $TMP_LIB/libparted-fs-resize.so.0
	chmod +x $TMP_LIB/libparted.so.2
	chmod +x $TMP_LIB/libblkid.so.1
	chmod +x $TMP_LIB/libuuid.so.1
	chmod +x $TMP_LIB/libpng16.so.16
	chmod +x $TMP_LIB/libncurses.so.6
	chmod +x $TMP_LIB/libmsettings.so
	
	# Copiar el script de redimensionamiento
	echo "Copiando el script de redimensionamiento..."
	cp "$SDCARD_PATH/Koriki/bin/resize_partition" /tmp/
	chmod +x /tmp/resize_partition
	
	# Ejecutar el script de redimensionamiento
	echo "Ejecutando el script de redimensionamiento..."
	setmon
	killall -9 main
	/tmp/resize_partition
	exit 0
else
	echo "La partición ya ha sido redimensionada anteriormente."
fi
}

setmon() {
	export MON_PATH="/tmp/mons/bin"

	if [ ! -d $MON_PATH ]; then
		mkdir -p $MON_PATH
		
		if [ -f "$SDCARD_PATH/Koriki/bin/batmon" ]; then
			rm -f "$SDCARD_PATH/Koriki/bin/batmon"
			sync
		fi
		if [ -f "$SDCARD_PATH/Koriki/bin/charging" ]; then
			rm -f "$SDCARD_PATH/Koriki/bin/charging"
			sync
		fi
		if [ -f "$SDCARD_PATH/Koriki/bin/keymon" ]; then
			rm -f "$SDCARD_PATH/Koriki/bin/keymon"
			sync
		fi
		if [ -f "$SDCARD_PATH/Koriki/bin/shutdown" ]; then
			rm -f "$SDCARD_PATH/Koriki/bin/shutdown"
			sync
		fi
		if [ -f "$SDCARD_PATH/Koriki/bin/killall" ]; then
			rm -f "$SDCARD_PATH/Koriki/bin/killall"
			sync
		fi
		
		cp "$SDCARD_PATH/Koriki/bin/sp/batmon" $MON_PATH/
		cp "$SDCARD_PATH/Koriki/bin/sp/charging" $MON_PATH/
		cp "$SDCARD_PATH/Koriki/bin/sp/keymon" $MON_PATH/
		cp "$SDCARD_PATH/Koriki/bin/sp/shutdown" $MON_PATH/
		cp "$SDCARD_PATH/Koriki/bin/sp/killall" $MON_PATH/
		
		chmod +x $MON_PATH/batmon
		chmod +x $MON_PATH/charging
		chmod +x $MON_PATH/keymon
		chmod +x $MON_PATH/shutdown
		chmod +x $MON_PATH/killall
		export PATH="${MON_PATH}:${PATH}"
	fi
}

killprocess() {
	pid=`ps | grep $1 | grep -v grep | cut -d' ' -f3`
	kill -9 $pid
}

init_lcd() {
	cat /proc/ls
	if [ $1 -ne 0 ] ; then
		sleep $1
	fi 
}

runifnecessary() {
	a=`ps | grep $1 | grep -v grep`
	if [ "$a" == "" ] ; then
		$2 &
	fi
}

reset_settings() {
	if [ -f "${SDCARD_PATH}/.reset_settings" ]; then
		if [ "$MODEL" == "MM" ]; then
			if [ -f "/appconfigs/system.json.old" ]; then
				if [ "$SUBMODEL" == "MMv4" ]; then
					cp "${SYSTEM_PATH}"/assets/system-v4.json "$SETTINGS_FILE"
				fi
				if [ "$SUBMODEL" == "MM" ]; then
					cp "${SYSTEM_PATH}"/assets/system.json "$SETTINGS_FILE"
				fi
			else
				cp "${SYSTEM_PATH}"/assets/system.json "$SETTINGS_FILE"
			fi

			cp "${SYSTEM_PATH}"/assets/last_state.sav "${SDCARD_PATH}"/.simplemenu/last_state.sav
			touch "${SDCARD_PATH}"/.simplemenu/default_section
			rm "${SDCARD_PATH}"/.reset_settings
			sync
			shutdown
			sleep 5
		else
			if [ "$SUBMODEL" == "MMFLIP" ]; then
				cp "${SYSTEM_PATH}"/assets/system.mmf.json "$SETTINGS_FILE"
			else
				cp "${SYSTEM_PATH}"/assets/system.mmp.json "$SETTINGS_FILE"
			fi

			cp "${SYSTEM_PATH}"/assets/last_state.sav "${SDCARD_PATH}"/.simplemenu/last_state.sav
			touch "${SDCARD_PATH}"/.simplemenu/default_section
			rm "${SDCARD_PATH}"/.reset_settings
			sync
			shutdown -r
			sleep 5
		fi
	fi
}

update() {
	
	echo "Checking for updater Koriki package"
	
	if [ -f "${SDCARD_PATH}"/.deletes ]; then
		while IFS= read -r file_to_delete; do
		rm -rf "${file_to_delete}"
		done < "${SDCARD_PATH}"/.deletes
		rm "${SDCARD_PATH}"/.deletes
	fi
	
	if [ -f "${SDCARD_PATH}/"update_koriki_*.zip ]; then
		
		echo "update Koriki package found"
		
		for file in `ls "${SDCARD_PATH}"/update_koriki_*.zip`; do
		unzip -q -o "${file}" ".update_splash.png" -d "${SDCARD_PATH}"
		sync
		
		"${SYSTEM_PATH}"/bin/show "${SDCARD_PATH}"/.update_splash.png
		
		unzip -q -o "${file}" ".deletes" -d "${SDCARD_PATH}"
		
		if [ -f "${SDCARD_PATH}"/.deletes ]; then
			while IFS= read -r file_to_delete; do
			if [ -f "${file_to_delete}" ]; then
				rm "${file_to_delete}"
			elif [ -d "${file_to_delete}" ]; then
				rm -rf "${file_to_delete}"
			fi
			done < "${SDCARD_PATH}"/.deletes
		fi
		
		unzip -q -o "${file}" -d "${SDCARD_PATH}"
		
		rm "${file}"
		
		if [ -f "${SDCARD_PATH}"/.deletes ]; then
			rm "${SDCARD_PATH}"/.deletes
		fi
		
		if [ -f "${SDCARD_PATH}"/.update_splash.png ]; then
			rm "${SDCARD_PATH}"/.update_splash.png
		fi
		
		sleep 5s
		done
		
		sync
		setmon
		shutdown
		
		sleep 10s
	fi
	
	echo "update Koriki package not found"
}

dir_scaffolding() {
	## Quake fbl's by @neilswann80
	pak="exec=pak0.pak"
	for dir2 in QUAKE/id1 QUAKE/hipnotic QUAKE/rogue QUAKE/dopa; do
		dir="/mnt/SDCARD/Roms/${dir2}"
		count=`ls -1 ${dir}/*.fbl 2>/dev/null | wc -l`
		if [ $count -eq 0 ] && [ -e "${dir}/pak0.pak" ]; then
			case ${dir2} in
				QUAKE/id1)      echo "$pak" > "${dir}/Quake.fbl" ;;
				QUAKE/hipnotic) echo "$pak" > "${dir}/Mission pack 1.fbl" ;;
				QUAKE/rogue)    echo "$pak" > "${dir}/Mission pack 2.fbl" ;;
				QUAKE/dopa)     echo "$pak" > "${dir}/Episode 5. Dimension of the Past.fbl" ;;
			esac
		fi
		done
}

get_screen_resolution() {
	max_attempts=10
	attempt=0
	
	echo "get_screen_resolution: start"
	while [ "$attempt" -lt "$max_attempts" ]; do
		screen_resolution=$(grep 'Current TimingWidth=' /proc/mi_modules/fb/mi_fb0 | sed 's/Current TimingWidth=\([0-9]*\),TimingWidth=\([0-9]*\),.*/\1x\2/')
		if [ -n "$screen_resolution" ]; then
			echo "get_screen_resolution: success, resolution: $screen_resolution"
			break
		fi
		echo "get_screen_resolution: attempt $attempt failed"
		attempt=$((attempt + 1))
		sleep 0.5
	done
	
	if [ -z "$screen_resolution" ]; then
		echo "get_screen_resolution: failed to get screen resolution, fall back to 640x480"
		touch /tmp/get_screen_resolution_failed
	fi
	
	if [ "$screen_resolution" = "752x560" ]; then
		touch /tmp/new_res_available
	else
		# can't use 752x560 without appropriate firmware or screen
		screen_resolution="640x480"
	fi
	
	echo -n "$screen_resolution" > /tmp/screen_resolution
}

change_resolution() {
	res_x=""
	res_y=""
	
	if [ -n "$1" ]; then
		res_x=$(echo "$1" | cut -d 'x' -f 1)
		res_y=$(echo "$1" | cut -d 'x' -f 2)
	else
		res_x=$(echo "$screen_resolution" | cut -d 'x' -f 1)
		res_y=$(echo "$screen_resolution" | cut -d 'x' -f 2)
	fi
	
	echo "change resolution to $res_x x $res_y"
	
	fbset -g "$res_x" "$res_y" "$res_x" "$((res_y * 3))" 32
}

reset_soundfix() {
	runsvr=`/customer/app/jsonval audiofix`
	FILE="/customer/app/axp_test"
	FILE2="/tmp/audioserver_on"

	if [ "$runsvr" != "0" ]; then
		touch /tmp/audioserver_on
		/mnt/SDCARD/Koriki/bin/audioserver &
		if [ -f /mnt/SDCARD/Koriki/lib/libpadsp.so ]; then
			export LD_PRELOAD=/mnt/SDCARD/Koriki/lib/libpadsp.so
		fi
	else #fixed slow music menu
		touch /tmp/audioserver_on
		/mnt/SDCARD/Koriki/bin/audioserver &
		if [ -f /mnt/SDCARD/Koriki/lib/libpadsp.so ]; then
			export LD_PRELOAD=/mnt/SDCARD/Koriki/lib/libpadsp.so
		fi
	
		if [ -f /mnt/SDCARD/Koriki/lib/libpadsp.so ]; then
			unset LD_PRELOAD
		fi
	
		if [ -f "$FILE" ]; then
			killall audioserver
			killall audioserver.plu
		else
			killall audioserver
			killall audioserver.min
		fi

		if [ -f "$FILE2" ]; then
			rm /tmp/audioserver_on
			/mnt/SDCARD/Koriki/bin/freemma
		fi
	fi
	sync
}

# set virtual memory size
echo 4096 > "/proc/sys/vm/max_map_count"

# Init_lcd
init_lcd 1

# Init backlight
echo 0 > "/sys/class/pwm/pwmchip0/export"
echo 800 > "/sys/class/pwm/pwmchip0/pwm0/period"
echo 70 > "/sys/class/pwm/pwmchip0/pwm0/duty_cycle"
echo 1 > "/sys/class/pwm/pwmchip0/pwm0/enable"

# Set screen resolution
get_screen_resolution
change_resolution

# Resize microsd
resize

# check swap size
if [ -f "${SWAPFILE}" ]; then
	SWAPSIZE=`stat -c %s "${SWAPFILE}"`
	
	MINSIZE=$((256 * 1024 * 1024))
	
	if [ "$SWAPSIZE" -lt "$MINSIZE" ]; then
		rm -f "${SWAPFILE}"
		sync
	fi
fi

# Enable swap
if [ ! -f "${SWAPFILE}" ]; then
	if [ -f "${SDCARD_PATH}/.reset_settings" ]; then
		"${SYSTEM_PATH}"/bin/show "${SYSTEM_PATH}"/images/swap.png
	fi
	dd if=/dev/zero of="${SWAPFILE}" bs=1M count=256
	mkswap "${SWAPFILE}"
	sync
	sleep 1
fi

if [ "$MODEL" == "MMP" ]; then
	swapon -p 40 "${SWAPFILE}"
else
	swapon -p 60 "${SWAPFILE}"
fi

# Update opportunity
update

# Set internal systems app in tmp
setmon

#kill main program from stock
killall -9 main

# Charge screen
charging

# Reset settings on first boot
reset_settings

# Make keys ssh
if [ "$MODEL" == "MMP" ]; then
	rsa_key="/appconfigs/dropbear_rsa_host_key"
	ecdsa_key="/appconfigs/dropbear_ecdsa_host_key"
	ed25519_key="/appconfigs/dropbear_ed25519_host_key"
	
	if [ ! -f "$rsa_key" ]; then
		dropbearkey -t rsa -f "$rsa_key"
	fi
	
	if [ ! -f "$ecdsa_key" ]; then
		dropbearkey -t ecdsa -f "$ecdsa_key"
	fi
	
	if [ ! -f "$ed25519_key" ]; then
		dropbearkey -t ed25519 -f "$ed25519_key"
	fi
fi

# Get save volumen
/customer/app/tinymix set 6 100
vol=`/customer/app/jsonval vol`

if [ "$vol" -ge "20" ]; then
	sed -i 's/"vol":\s*\([2][123]\)/"vol": 20/' "$SETTINGS_FILE"
	sync
	vol=`/customer/app/jsonval vol`
fi

vol=$((($vol*3)+40))
/customer/app/tinymix set 6 "$vol"

# Show bootScreen or videosplash
if [ -f "${SYSTEM_PATH}"/videosplash.mp4 ]; then
	echo 0 > "/sys/module/gpio_keys_polled/parameters/button_enable"
	"${SYSTEM_PATH}"/bin/ffplayer "${SYSTEM_PATH}"/videosplash.mp4
	echo 1 > "/sys/module/gpio_keys_polled/parameters/button_enable"
else
	"${SYSTEM_PATH}"/bin/show "${SDCARD_PATH}"/.simplemenu/resources/loading.png
fi

# Create ROMs scaffolding
dir_scaffolding

# fix if the settings files is missing
if [ ! -f "$SETTINGS_FILE" ]; then
	if [ "$MODEL" == "MMP" ]; then
		cp "${SYSTEM_PATH}"/assets/system.mmp.json "$SETTINGS_FILE"
		sync
		shutdown -r
		sleep 5
	fi
	if [ "$SUBMODEL" == "MMFLIP" ]; then
		cp "${SYSTEM_PATH}"/assets/system.mmf.json "$SETTINGS_FILE"
		sync
		shutdown -r
		sleep 5
	fi

	if [ "$MODEL" == "MM" ]; then
		if [ "$SUBMODEL" == "MM" ]; then
			cp "${SYSTEM_PATH}"/assets/system.json "$SETTINGS_FILE"
		fi
		if [ "$SUBMODEL" == "MMv4" ]; then
			cp "${SYSTEM_PATH}"/assets/system-v4.json "$SETTINGS_FILE"
		fi
		sync
		shutdown
		sleep 5
	fi
fi

# Latency reduction audioserver by Eggs
# NOTE: could cause performance issues on more demanding cores...maybe?

# check audiofix
runsvr=`/customer/app/jsonval audiofix`

if [ "$runsvr" != "0" ]; then
	#sed -i "s/\"audiofix\":\s*[01]/\"audiofix\": 0/" "$SETTINGS_FILE"
	/mnt/SDCARD/Koriki/bin/audioserver &
	touch /tmp/audioserver_on
	sync
	export LD_PRELOAD=/mnt/SDCARD/Koriki/lib/libpadsp.so
fi

# check if wifi value is ON or OFF
if [ "$MODEL" == "MMP" ]; then
	filewifi="/mnt/SDCARD/.simplemenu/wifi.sav"
	chain="ssid=\"MiyooMini\""
	ESSID=$(sed -n 's/^\s*ssid="\([^"]*\)".*/\1/p' /appconfigs/wpa_supplicant.conf)
	runwifi=`/customer/app/jsonval wifi`
	
	if [ -f "$filewifi" ]; then
		if ! grep -q "$chain" "/appconfigs/wpa_supplicant.conf"; then
			if grep -q "wifi: on" "$filewifi"; then
				if grep -q "ssid=" "/appconfigs/wpa_supplicant.conf"; then
					echo "autowifi is on"
					echo "connecting to the last network..."
					if [ "$runwifi" == "0" ]; then
						sed -i "s/\"wifi\":\s*[01]/\"wifi\": 1/" "$SETTINGS_FILE"
						/customer/app/axp_test wifion
						sync
					else
						/customer/app/axp_test wifion
					fi
					
					sleep 1.5
					echo "activating wifi..."
					/sbin/ifconfig wlan0 up
					
					sleep 0.5
					
					if /mnt/SDCARD/Koriki/bin/iwlist wlan0 scan | grep -q "$ESSID"; then
						echo "ssid found!"
						echo "enable autowifi"
						sleep 0.5
						/mnt/SDCARD/Koriki/bin/wpa_supplicant -B -D nl80211 -i wlan0 -c /appconfigs/wpa_supplicant.conf
						/sbin/udhcpc -i wlan0 -s /etc/init.d/udhcpc.script &
						echo "connecting..."
						while true; do
							if /sbin/ifconfig wlan0 | grep -q "inet addr"; then
								echo "connected!"
								/mnt/SDCARD/Koriki/bin/ntpdate -u pool.ntp.org
								echo "sinc network time"
								break
							else
								sleep 2
							fi
						done &
					elif ! /mnt/SDCARD/Koriki/bin/iwlist wlan0 scan | grep -q "$ESSID"; then
						echo "ssid not found..."
						echo "disable autowifi"
						sleep 0.5
						/sbin/ifconfig wlan0 down
						echo "power off wifi..."
						/customer/app/axp_test wifioff &
						sed -i "s/\"wifi\":\s*[01]/\"wifi\": 0/" "$SETTINGS_FILE"
						sync
					fi
				fi
				
				if ! grep -q "ssid=" "/appconfigs/wpa_supplicant.conf"; then
					if [ "$runwifi" == "1" ]; then
						echo "last network not save"
						/customer/app/axp_test wifioff &
						sed -i "s/\"wifi\":\s*[01]/\"wifi\": 0/" "$SETTINGS_FILE"
						sync
					fi
				fi
				
			elif grep -q "wifi: off" "$filewifi"; then
				if [ "$runwifi" == "1" ]; then
					echo "autowifi is off"
					/customer/app/axp_test wifioff &
					sed -i "s/\"wifi\":\s*[01]/\"wifi\": 0/" "$SETTINGS_FILE"
					sync
				fi
			fi
		elif grep -q "$chain" "/appconfigs/wpa_supplicant.conf"; then
			if [ "$runwifi" == "1" ]; then
				/customer/app/axp_test wifioff &
				sed -i "s/\"wifi\":\s*[01]/\"wifi\": 0/" "$SETTINGS_FILE"
				sync
			fi
		fi
	else #file wifi.sav not found
		if [ "$runwifi" == "1" ]; then
			/customer/app/axp_test wifioff &
			sed -i "s/\"wifi\":\s*[01]/\"wifi\": 0/" "$SETTINGS_FILE"
			sync
		fi
	fi
fi     

# autochange retroarch version and show/quit networks app

if [ ! -f /customer/app/axp_test ]; then
	rm "${RETROARCH_PATH}"/retroarch
	if [ "$SUBMODEL" == "MM" ]; then
		cp "${RETROARCH_PATH}"/retroarch.mini "${RETROARCH_PATH}"/retroarch
		sed -i 's|^video_fullscreen_x = "752"$|video_fullscreen_x = "640"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^video_fullscreen_y = "560"$|video_fullscreen_y = "480"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^video_windowed_position_width = "752"$|video_windowed_position_width = "640"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^video_windowed_position_height = "560"$|video_windowed_position_height = "480"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^video_window_auto_width_max = "752"$|video_window_auto_width_max = "640"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^video_window_auto_height_max = "560"$|video_window_auto_height_max = "480"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^custom_viewport_width = "752"$|custom_viewport_width = "640"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^custom_viewport_height = "560"$|custom_viewport_height = "480"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^input_overlay = ":/.retroarch/overlay/CTR/Perfect_CRT-560p.cfg"$|input_overlay = ":/.retroarch/overlay/CTR/Perfect_CRT-240p.cfg"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
	fi
	
	if [ "$SUBMODEL" == "MMv4" ]; then
		cp "${RETROARCH_PATH}"/retroarch.miniv4 "${RETROARCH_PATH}"/retroarch
		sed -i 's|^video_fullscreen_x = "640"$|video_fullscreen_x = "752"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^video_fullscreen_y = "480"$|video_fullscreen_y = "560"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^video_windowed_position_width = "640"$|video_windowed_position_width = "752"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^video_windowed_position_height = "480"$|video_windowed_position_height = "560"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^video_window_auto_width_max = "640"$|video_window_auto_width_max = "752"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^video_window_auto_height_max = "480"$|video_window_auto_height_max = "560"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^custom_viewport_width = "640"$|custom_viewport_width = "752"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^custom_viewport_height = "480"$|custom_viewport_height = "560"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^input_overlay = ":/.retroarch/overlay/CTR/Perfect_CRT-240p.cfg"$|input_overlay = ":/.retroarch/overlay/CTR/Perfect_CRT-560p.cfg"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
	fi
	sync
	if [ -f "${SDCARD_PATH}"/.simplemenu/apps/Ftp.sh ]; then
		mv "${SDCARD_PATH}"/.simplemenu/apps/Ftp.sh "${SDCARD_PATH}"/.simplemenu/apps/Ftp
		sync
	fi
	if [ -f "${SDCARD_PATH}"/.simplemenu/apps/Ssh.sh ]; then
		mv "${SDCARD_PATH}"/.simplemenu/apps/Ssh.sh "${SDCARD_PATH}"/.simplemenu/apps/Ssh
		sync
	fi
else
	rm "${RETROARCH_PATH}"/retroarch
	if [ "$SUBMODEL" == "MMFLIP" ]; then
		cp "${RETROARCH_PATH}"/retroarch.miniflip "${RETROARCH_PATH}"/retroarch
		sed -i 's|^video_fullscreen_x = "640"$|video_fullscreen_x = "752"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^video_fullscreen_y = "480"$|video_fullscreen_y = "560"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^video_windowed_position_width = "640"$|video_windowed_position_width = "752"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^video_windowed_position_height = "480"$|video_windowed_position_height = "560"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^video_window_auto_width_max = "640"$|video_window_auto_width_max = "752"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^video_window_auto_height_max = "480"$|video_window_auto_height_max = "560"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^custom_viewport_width = "640"$|custom_viewport_width = "752"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^custom_viewport_height = "480"$|custom_viewport_height = "560"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^input_overlay = ":/.retroarch/overlay/CTR/Perfect_CRT-240p.cfg"$|input_overlay = ":/.retroarch/overlay/CTR/Perfect_CRT-560p.cfg"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sync
	else
		cp "${RETROARCH_PATH}"/retroarch.plus "${RETROARCH_PATH}"/retroarch
		sed -i 's|^video_fullscreen_x = "752"$|video_fullscreen_x = "640"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^video_fullscreen_y = "560"$|video_fullscreen_y = "480"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^video_windowed_position_width = "752"$|video_windowed_position_width = "640"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^video_windowed_position_height = "560"$|video_windowed_position_height = "480"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^video_window_auto_width_max = "752"$|video_window_auto_width_max = "640"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^video_window_auto_height_max = "560"$|video_window_auto_height_max = "480"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^custom_viewport_width = "752"$|custom_viewport_width = "640"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^custom_viewport_height = "560"$|custom_viewport_height = "480"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sed -i 's|^input_overlay = ":/.retroarch/overlay/CTR/Perfect_CRT-560p.cfg"$|input_overlay = ":/.retroarch/overlay/CTR/Perfect_CRT-240p.cfg"|' "${RETROARCH_PATH}/.retroarch/retroarch.cfg"
		sync
	fi
	if [ -f "${SDCARD_PATH}"/.simplemenu/apps/Ftp ]; then
		mv "${SDCARD_PATH}"/.simplemenu/apps/Ftp "${SDCARD_PATH}"/.simplemenu/apps/Ftp.sh
		sync
	fi
	if [ -f "${SDCARD_PATH}"/.simplemenu/apps/Ssh ]; then
		mv "${SDCARD_PATH}"/.simplemenu/apps/Ssh "${SDCARD_PATH}"/.simplemenu/apps/Ssh.sh
		sync
	fi
fi

if [ -f /tmp/new_res_available ]; then
	if grep -q "v1" "${RETROARCH_PATH}"/done; then
			rm "${RETROARCH_PATH}"/done
			touch "${RETROARCH_PATH}"/done
			echo "v4" > "${RETROARCH_PATH}"/done
    		find /mnt/SDCARD/RetroArch/.retroarch/config/ -type f -name "*.cfg" | while read file; do
        	if ! grep -q 'video_scale_integer = "true"' "$file"; then
            	if grep -q "video_filter = \":/.retroarch/filters/video/Grid3x.filt\"" "$file"; then
                	sed -i 's|video_filter = ":/.retroarch/filters/video/Grid3x.filt"|video_filter = ":/.retroarch/filters/video/Grid2x.filt"|g' "$file"
            	fi
        	fi
    		done
		sync
	fi
else
	if grep -q "v4" "${RETROARCH_PATH}"/done; then
			rm "${RETROARCH_PATH}"/done
			touch "${RETROARCH_PATH}"/done
			echo "v1" > "${RETROARCH_PATH}"/done
    		find /mnt/SDCARD/RetroArch/.retroarch/config/ -type f -name "*.cfg" | while read file; do
        	if ! grep -q 'video_scale_integer = "true"' "$file"; then
            	if grep -q "video_filter = \":/.retroarch/filters/video/Grid2x.filt\"" "$file"; then
                	sed -i 's|video_filter = ":/.retroarch/filters/video/Grid2x.filt"|video_filter = ":/.retroarch/filters/video/Grid3x.filt"|g' "$file"
            	fi
        	fi
    		done
		sync
	fi
fi

# Set the last CPU and GOV change
if [ -f "${CPUSAVE}" ]; then
	CPU=`cat "${CPUSAVE}"`
	echo "${CPU}" > "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq"
fi
if [ ! -f "${CPUSAVE}" ]; then
	touch "${CPUSAVE}"
	echo 1200000 > "${CPUSAVE}"
	sync
fi
if [ -f "${GOVSAVE}" ]; then
	echo ondemand > "${GOVSAVE}"
	sync
fi
if [ ! -f "${GOVSAVE}" ]; then
	touch "${GOVSAVE}"
	echo ondemand > "${GOVSAVE}"
	sync
fi
if [ -f "${SPEEDSAVE}" ]; then
	echo "<unsupported>" > "${SPEEDSAVE}"
	sync
fi
if [ ! -f "${SPEEDSAVE}" ]; then
	touch "${SPEEDSAVE}"
	echo "<unsupported>" > "${SPEEDSAVE}"
	sync
fi

# Koriki keymon
runifnecessary "keymon" keymon

# Koriki batmon
runifnecessary "batmon" batmon

# create dhcp.leases
if [ ! -f /appconfigs/dhcp.leases ]; then
	touch /appconfigs/dhcp.leases
	sync
fi

# fix simplemenu bootloop
if [ ! -f "${SDCARD_PATH}"/.simplemenu/favorites.sav ]; then
	touch "${SDCARD_PATH}"/.simplemenu/favorites.sav
	sync
fi

# clear ftp log file
if [ -f "${SYSTEM_PATH}"/bftpdutmp ]; then
	rm "${SYSTEM_PATH}"/bftpdutmp
	touch "${SYSTEM_PATH}"/bftpdutmp
	sync
fi

# Detect 'B' press to fix SM boot loops
if cat /sys/kernel/debug/gpio|grep "gpio-6 "|grep lo > /dev/null 2>&1
then
	rm  "${SDCARD_PATH}"/.simplemenu/last_state.sav
	sync
fi

#check FBNEO folder
if [ -d "/mnt/SDCARD/Roms/FBN" ]; then
	mv "/mnt/SDCARD/Roms/FBN" "/mnt/SDCARD/Roms/FBNEO"
fi

# Detect if networks app was the last app and erese this from SM if is the model MM.
if [ "$MODEL" == "MM" ]; then
	if [ -f "${SDCARD_PATH}/.simplemenu/default_section" ]; then
    	ACTIVE="default"
	elif [ -f "${SDCARD_PATH}/.simplemenu/alphabetic_section" ]; then
    	ACTIVE="alphabetic"
	elif [ -f "${SDCARD_PATH}/.simplemenu/systems_section" ]; then
    	ACTIVE="systems"
	else
    	ACTIVE="default"
	fi

	LAST="${SDCARD_PATH}/.simplemenu/last_state.sav"
	
	case "$ACTIVE" in

    default)
        sed -i '
        s/^1;1;0;14;14;/1;1;0;12;12;/
        s/^1;1;1;6;14;/1;1;1;4;12;/
        s/^1;1;1;2;14;/1;1;1;0;12;/
        s/^1;1;1;1;14;/1;1;1;0;12;/
        s/^1;1;1;0;14;/1;1;1;0;12;/

        s/^0;1;0;14;14;/0;1;0;12;12;/
        s/^0;1;1;6;14;/0;1;1;4;12;/
        s/^0;1;1;2;14;/0;1;1;0;12;/
        s/^0;1;1;1;14;/0;1;1;0;12;/
        s/^0;1;1;0;14;/0;1;1;0;12;/
        ' "$LAST"
    ;;

    alphabetic)
        sed -i '
        s/^1;2;0;14;14;/1;2;0;12;12;/
        s/^1;2;1;6;14;/1;2;1;4;12;/
        s/^1;2;1;2;14;/1;2;1;0;12;/
        s/^1;2;1;1;14;/1;2;1;0;12;/
        s/^1;2;1;0;14;/1;2;1;0;12;/

        s/^0;2;0;14;14;/0;2;0;12;12;/
        s/^0;2;1;6;14;/0;2;1;4;12;/
        s/^0;2;1;2;14;/0;2;1;0;12;/
        s/^0;2;1;1;14;/0;2;1;0;12;/
        s/^0;2;1;0;14;/0;2;1;0;12;/
        ' "$LAST"
    ;;

    systems)
        sed -i '
        s/^1;0;0;14;14;/1;0;0;12;12;/
        s/^1;0;1;6;14;/1;0;1;4;12;/
        s/^1;0;1;2;14;/1;0;1;0;12;/
        s/^1;0;1;1;14;/1;0;1;0;12;/
        s/^1;0;1;0;14;/1;0;1;0;12;/

        s/^0;0;0;14;14;/0;0;0;12;12;/
        s/^0;0;1;6;14;/0;0;1;4;12;/
        s/^0;0;1;2;14;/0;0;1;0;12;/
        s/^0;0;1;1;14;/0;0;1;0;12;/
        s/^0;0;1;0;14;/0;0;1;0;12;/
        ' "$LAST"
    ;;
    esac
	sync
fi

# Change speed in Drastic
if [ -f "${SDCARD_PATH}"/App/drastic/launch.sh ]; then
	if [ "$MODEL" == "MM" ]; then
		CURRENT_MAXCPU=$(grep -o '"maxcpu":[0-9]*' "${SDCARD_PATH}"/App/drastic/resources/settings.json | awk -F':' '{print $2}')
		if [ "$CURRENT_MAXCPU" != "1500" ]; then
			sed -i 's/"maxcpu":'"$CURRENT_MAXCPU"'/"maxcpu":1500/' "${SDCARD_PATH}"/App/drastic/resources/settings.json
			sed -i 's|"${SYSTEM_PATH}"/bin/cpuclock '"$CURRENT_MAXCPU"'|"${SYSTEM_PATH}"/bin/cpuclock 1500|' "${SDCARD_PATH}"/App/drastic/launch.sh
			sync
		fi
	else
		CURRENT_MAXCPU=$(grep -o '"maxcpu":[0-9]*' "${SDCARD_PATH}"/App/drastic/resources/settings.json | awk -F':' '{print $2}')
		if [ "$CURRENT_MAXCPU" != "1600" ]; then
			sed -i 's/"maxcpu":'"$CURRENT_MAXCPU"'/"maxcpu":1600/' "${SDCARD_PATH}"/App/drastic/resources/settings.json
			sed -i 's|"${SYSTEM_PATH}"/bin/cpuclock '"$CURRENT_MAXCPU"'|"${SYSTEM_PATH}"/bin/cpuclock 1600|' "${SDCARD_PATH}"/App/drastic/launch.sh
			sync
		fi
	fi
fi

# clean pico8 logs files
if [ -f "${SDCARD_PATH}"/App/pico/.lexaloffle/pico-8/activity_log.txt ]; then
	rm "${SDCARD_PATH}"/App/pico/.lexaloffle/pico-8/activity_log.txt
	rm "${SDCARD_PATH}"/App/pico/.lexaloffle/pico-8/log.txt
fi

# Set time
if dmesg|fgrep -q "power key is on"; then
	export SUBMODEL="MMP_RTC"
else
	if [ "$SUBMODEL" == "MMFLIP" ]; then
		export SUBMODEL="MMP_RTC"
	else
		export SUBMODEL="MMP_NO_RTC"
	fi
fi

if [ -f "${SDCARD_PATH}"/App/Clock/time.txt ]; then
	localtime=`cat "${SDCARD_PATH}"/App/Clock/time.txt`
	if dmesg|fgrep -q "Please set rtc timer (hwclock -w) "; then
		hwclock -w
		date -s "${localtime}"
	else
		if [ "$SUBMODEL" == "MMP_NO_RTC" ]; then
			date -s "${localtime}"
		fi
	fi
else
	touch "${SDCARD_PATH}"/App/Clock/time.txt
	hwclock -w
	date -s "2025-09-01 10:00:00"
	localtime=$(date +"%Y-%m-%d %T")
	echo "$localtime" > "${SDCARD_PATH}"/App/Clock/time.txt
fi

if [ -f "${SDCARD_PATH}"/App/Clock/timezone.txt ]; then
	timezone=`cat "${SDCARD_PATH}"/App/Clock/timezone.txt`
	export TZ=UTC$((-1*timezone))
else
	touch "${SDCARD_PATH}"/App/Clock/timezone.txt
	echo +0 > "${SDCARD_PATH}"/App/Clock/timezone.txt
	export TZ=UTC+0
fi

#kill telnetd

killall -15 telnetd

# Remount passwd/group to add our own users

if [ "$MODEL" == "MMP" ]; then
	mount -o bind "${SYSTEM_PATH}"/etc/passwd /etc/passwd
	mount -o bind "${SYSTEM_PATH}"/etc/group /etc/group
	mount -o bind "${SYSTEM_PATH}"/etc/profile /etc/profile
fi

#scrap

if [ ! -f "/appconfigs/keyscraper.txt" ]; then
   touch /appconfigs/keyscraper.txt
   echo uhdsjndoujahfjdnfgjdfunsaofugasufaslonf > /appconfigs/keyscraper.txt
fi

# Launch SimpleMenu

echo "Welcome to Koriki CFW"

while [ 1 ]; do
	HOME="${SDCARD_PATH}"
	cd "${SDCARD_PATH}"/.simplemenu
	change_resolution
	reset_soundfix
	
	./simplemenu
	
	sleep 4s
	sync
done

# turf off the console.
sync
sleep 5
shutdown
