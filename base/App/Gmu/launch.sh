#!/bin/sh

my_dir=`dirname $0`
runsvr=`/customer/app/jsonval audiofix`

if [ "$runsvr" != "0" ] ; then
	FILE=/customer/app/axp_test

    if [ -f "$FILE" ]; then
        killall audioserver
		killall audioserver.plu
		FILE2=/tmp/audioserver_on
		if [ -f "$FILE2" ]; then
			rm /tmp/audioserver_on
			/customer/app/sysmon freemma
		fi
    else
        killall audioserver
		killall audioserver.min
    fi
fi

cd $my_dir
./launch2.sh

runsvr=`/customer/app/jsonval audiofix`
if [ "$runsvr" != "0" ] ; then
	FILE=/customer/app/axp_test
	if [ -f "$FILE" ]; then
	touch /tmp/audioserver_on
	fi
	/mnt/SDCARD/Koriki/bin/audioserver &
	if [ -f /customer/lib/libpadsp.so ]; then
		export LD_PRELOAD=/customer/lib/libpadsp.so
	fi
fi