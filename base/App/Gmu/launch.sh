#!/bin/sh

my_dir=`dirname $0`
runsvr=`/customer/app/jsonval audiofix`

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
			/mnt/SDCARD/Koriki/bin/freemma > NUL
		fi
    else
        killall audioserver
		killall audioserver.min
		FILE2=/tmp/audioserver_on
		if [ -f "$FILE2" ]; then
			rm /tmp/audioserver_on
			/mnt/SDCARD/Koriki/bin/freemma > NUL
		fi
    fi
fi

cd $my_dir
./launch2.sh

runsvr=`/customer/app/jsonval audiofix`
if [ "$runsvr" != "0" ] ; then
	touch /tmp/audioserver_on
	/mnt/SDCARD/Koriki/bin/audioserver &
	if [ -f /mnt/SDCARD/Koriki/lib/libpadsp.so ]; then
		export LD_PRELOAD=/mnt/SDCARD/Koriki/lib/libpadsp.so
	fi
fi