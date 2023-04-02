#!/bin/bash

SM_SRC=simplemenu/simplemenu/output/simplemenu
SM_DES=.simplemenu/simplemenu
INVOKER_SRC=invoker/invoker/invoker.dge
INVOKER_DES=.simplemenu/invoker.dge
RA_SRC=RetroArch/retroarch
RA_DES=RetroArch/retroarch
AUDIOSERVER_SRC=latency_reduction/src/audioserver.min
AUDIOSERVER_DES=Koriki/bin/audioserver.min
COMMANDER_SRC=Commander_Italic_rev4/sources/output/DinguxCommander
COMMANDER_DES=App/Commander_Italic/DinguxCommander
GMU_SRC=gmu/gmu.bin
GMU_DES=App/Gmu/gmu.bin
BOOTSCRENSEL_SRC=bootScreenSelector/bootScreenSelector
BOOTSCRENSEL_DES=App/Bootscreen_Selector/bootScreenSelector
SYSINFO_SRC=systemInfo/systemInfo
SYSINFO_DES=App/System_Info/systemInfo
CHARGING_SRC=charging/charging
CHARGING_DES=Koriki/bin/charging
KEYMON_SRC=keymon/keymon
KEYMON_DES=Koriki/bin/keymon
SHOWSCREEN_SRC=showScreen/show
SHOWSCREEN_DES=Koriki/bin/show

if [ $# -eq 0 ]
then
  echo "Usage: ./make_release.sh VERSION"
  exit
fi


DIRECTORY=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)
cd ${DIRECTORY}

for binary in "${SM_SRC};${SM_DES}" "${INVOKER_SRC};${INVOKER_DES}" "${RA_SRC};${RA_DES}" "${AUDIOSERVER_SRC};${AUDIOSERVER_DES}" "${COMMANDER_SRC};${COMMANDER_DES}" "${GMU_SRC};${GMU_DES}" "${BOOTSCRENSEL_SRC};${BOOTSCRENSEL_DES}" "${SYSINFO_SRC};${SYSINFO_DES}" "${CHARGING_SRC};${CHARGING_DES}" "${KEYMON_SRC};${KEYMON_DES}" "${SHOWSCREEN_SRC};${SHOWSCREEN_DES}"
do
    bin_source=`echo ${binary}|cut -d ';' -f 1`
    bin_destiny=`echo ${binary}|cut -d ';' -f 2`
    if [ ! -f src/${bin_source} ]; then
        echo "The binary src/${bin_source} is missing. Compile it first."
        exit 1
    fi
    cp src/${bin_source} base/${bin_destiny}
done

cd base
echo ${1} > Koriki/version.txt
zip -9 -x '*.gitignore' -x '*.git*' -r ../update_koriki_${1}.zip .
rm Koriki/version.txt

for bin_destiny in "${SM_DES}" "${INVOKER_DES}" "${RA_DES}" "${AUDIOSERVER_DES}" "${COMMANDER_DES}" "${GMU_DES}" "${BOOTSCRENSEL_DES}" "${SYSINFO_DES}" "${CHARGING_DES}" "${KEYMON_DES}" "${SHOWSCREEN_DES}"
do
    rm ${bin_destiny}
done