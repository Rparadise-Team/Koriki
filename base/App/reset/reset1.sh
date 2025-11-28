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
	if dmesg|fgrep -q "FB_WIDTH=752"; then
		export MODEL="MMF"
	else
    	export MODEL="MMP"
	fi
fi

reset_settings() {
		cp ${SYSTEM_PATH}/assets/retroarch.cfg $SDCARD_PATH/RetroArch/.retroarch/retroarch.cfg
		sync

        if [ $MODEL = "MM" ]; then
		    if [ -f /appconfigs/system.json.old ]; then
			cp ${SYSTEM_PATH}/assets/system-v4-stock.json $SETTINGS_FILE
			else
			cp ${SYSTEM_PATH}/assets/system-stock.json $SETTINGS_FILE
			fi
            sync
			shutdown
            sleep 10s
		fi
        
		if [ $MODEL = "MMv4" ]; then
			cp ${SYSTEM_PATH}/assets/system-v4-stock.json $SETTINGS_FILE
			sync
			shutdown
            sleep 10s
		fi
		
		if [ $MODEL = "MMP" ]; then
            cp ${SYSTEM_PATH}/assets/system.mmp-stock.json $SETTINGS_FILE
            sync
            shutdown -r
            sleep 10s
        fi
		
		if [ $MODEL = "MMF" ]; then
            cp ${SYSTEM_PATH}/assets/system.mmf-stock.json $SETTINGS_FILE
            sync
            shutdown -r
            sleep 10s
        fi
}

reset_settings
