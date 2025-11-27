#!/bin/sh
echo $0 $*
cd $(dirname "$0")
HOME=/mnt/SDCARD
./OverlaySelector overlay-GBC-DoubleCherryGB.cfg
sync
