#!/bin/sh
echo $0 $*
cd $(dirname "$0")
HOME=/mnt/SDCARD
./OverlaySelector overlay-WS-Beetle_WonderSwan.cfg
sync
