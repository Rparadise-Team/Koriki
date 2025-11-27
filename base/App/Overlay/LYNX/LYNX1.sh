#!/bin/sh
echo $0 $*
cd $(dirname "$0")
HOME=/mnt/SDCARD
./OverlaySelector overlay-LYNX-Beetle_Lynx.cfg
sync
