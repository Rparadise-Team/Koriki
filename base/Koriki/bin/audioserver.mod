#!/bin/sh
FILE=/customer/app/axp_test
cd `dirname "$0"`

if [ -f "$FILE" ]; then
LD_PRELOAD="./as_preload.so" ./audioserver.plu
else
LD_PRELOAD="./as_preload.so" ./audioserver.min
fi
