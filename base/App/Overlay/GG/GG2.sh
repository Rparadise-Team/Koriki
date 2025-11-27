#!/bin/sh
echo $0 $*
cd $(dirname "$0")
HOME=/mnt/SDCARD
./OverlaySelector overlay-GG-Genesis_Plus_GX.cfg
sync
