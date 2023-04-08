#!/bin/sh

my_dir=`dirname $0`

killall audioserver
killall audioserver.min
killall audioserver.plu

cd $my_dir
./launch2.sh

runsvr=`/customer/app/jsonval audiofix`
if [ "$runsvr" != "0" ] ; then
    /mnt/SDCARD/Koriki/bin/audioserver &
    if [ -f /customer/lib/libpadsp.so ]; then
        export LD_PRELOAD=/customer/lib/libpadsp.so
    fi
fi
