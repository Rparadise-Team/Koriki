#!/bin/sh
echo $0 $*
cd $(dirname "$0")
HOME=/mnt/SDCARD
export SDCARD_PATH=/mnt/SDCARD
export SYSTEM_PATH=${SDCARD_PATH}/Koriki
SETTINGS_INT_FILE="/appconfigs/system.json"
SETTINGS_EXT_FILE="/mnt/SDCARD/system.json"

# Detect flash type
if dmesg|fgrep -q "[FSP] Flash is detected (0x1100, 0x68, 0x40, 0x18) ver1.1"; then
    export SETTINGS_FILE=$SETTINGS_EXT_FILE
else
    if [ -f /appconfigs/system.json.old ]; then
    	export SETTINGS_FILE=$SETTINGS_EXT_FILE
	else
    	export SETTINGS_FILE=$SETTINGS_INT_FILE
	fi
fi

# Detect model
if [ ! -f /customer/app/axp_test ]; then
	if dmesg|fgrep -q "FB_WIDTH=752"; then
    	export MODEL="MMv4"
	else
		export MODEL="MM"
	fi
else
    export MODEL="MMP"
fi

reset_settings() {
        if [ $MODEL = "MM" ]; then
		    if [ -f /appconfigs/system.json.old ]; then
			cp ${SYSTEM_PATH}/assets/system-v4_old.json $SETTINGS_FILE
			else
			cp ${SYSTEM_PATH}/assets/system.json $SETTINGS_FILE
			fi
            sync
			reboot
            sleep 10s
		fi
        
		if [ $MODEL = "MMv4" ]; then
			cp ${SYSTEM_PATH}/assets/system-v4.json $SETTINGS_FILE
			sync
			reboot
            sleep 10s
		fi
		
		if [ $MODEL = "MMP" ]; then
            cp ${SYSTEM_PATH}/assets/system.mmp.json $SETTINGS_FILE
            sync
            reboot
            sleep 10s
        fi
}

reset_settings
