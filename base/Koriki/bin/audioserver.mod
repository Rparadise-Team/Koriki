#!/bin/sh
FILE=/customer/app/axp_test

if [ -f "$FILE" ]; then
LD_PRELOAD="/mnt/SDCARD/Koriki/lib/as_preload.so" /mnt/SDCARD/Koriki/bin/audioserver.plu
else
LD_PRELOAD="/mnt/SDCARD/Koriki/lib/as_preload.so" /mnt/SDCARD/Koriki/bin/audioserver.min
fi
