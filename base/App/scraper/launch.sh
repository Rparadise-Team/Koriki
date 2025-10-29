#!/bin/sh
cd $(dirname "$0")
export HOME=/mnt/SDCARD

python ./scraper_py27.pyc
sync
