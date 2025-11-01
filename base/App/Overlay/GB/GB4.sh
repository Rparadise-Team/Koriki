#!/bin/sh
echo $0 $*
cd $(dirname "$0")
HOME=/mnt/SDCARD
./OverlaySelector-GB-DoubleCherryGB
sync

if [ -f /tmp/new_res_available ]; then
    find /mnt/SDCARD/RetroArch/.retroarch/config/ -type f -name "*.cfg" | while read file; do
        if ! grep -q 'video_scale_integer = "true"' "$file"; then
            if grep -q "video_filter = \":/.retroarch/filters/video/Grid3x.filt\"" "$file"; then
                sed -i 's|video_filter = ":/.retroarch/filters/video/Grid3x.filt"|video_filter = ":/.retroarch/filters/video/Grid2x.filt"|g' "$file"
            fi
        fi
    done
    sync
fi
