#!/bin/sh
FILE=/customer/app/axp_test

if [ -f "$FILE" ]; then
/mnt/SDCARD/Koriki/bin/audioserver.plu > /dev/null 2 >&1
else
/mnt/SDCARD/Koriki/bin/audioserver.min > /dev/null 2 >&1
fi
