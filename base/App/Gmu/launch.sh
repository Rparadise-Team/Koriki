#!/bin/sh

my_dir=`dirname $0`

killall audioserver.mod

cd $my_dir
./launch2.sh

runsvr=`/customer/app/jsonval audiofix`
if [ "$runsvr" != "0" ] ; then
    LD_PRELOAD=/mnt/SDCARD/Koriki/lib/as_preload.so /mnt/SDCARD/Koriki/bin/audioserver.mod &
    if [ -f /customer/lib/libpadsp.so ]; then
        export LD_PRELOAD=/customer/lib/libpadsp.so
    fi
fi
