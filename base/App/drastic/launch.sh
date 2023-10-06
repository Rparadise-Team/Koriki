#!/bin/sh
mydir=`dirname "$0"`

export HOME=$mydir
export PATH=$mydir:$PATH
export LD_LIBRARY_PATH=$mydir/libs:$LD_LIBRARY_PATH
export SDL_VIDEODRIVER=mmiyoo
export SDL_AUDIODRIVER=mmiyoo
export EGL_VIDEODRIVER=mmiyoo

CUST_LOGO=1
CUST_CPUCLOCK=1

cd $mydir
if [ "$CUST_LOGO" == "1" ]; then
    echo "convert resources/logo/0.png to drastic_logo_0.raw"
    echo "convert resources/logo/1.png to drastic_logo_1.raw"
    ./png2raw
fi

sv=`cat /proc/sys/vm/swappiness`

# 60 by default
echo 10 > /proc/sys/vm/swappiness

cd $mydir

if [ "$CUST_CPUCLOCK" == "1" ]; then
    echo "set customized cpuspeed"
    /mnt/SDCARD/Koriki/bin/cpuclock 1600
fi

./drastic "$1"

if [ "$CUST_CPUCLOCK" == "1" ]; then
    echo "set customized cpuspeed"
    /mnt/SDCARD/Koriki/bin/cpuclock 1200
fi

echo $sv > /proc/sys/vm/swappiness
