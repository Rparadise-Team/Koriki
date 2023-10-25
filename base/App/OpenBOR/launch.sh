#!/bin/sh
HOME=`dirname "$0"`

export PATH=$HOME
export LD_LIBRARY_PATH=$HOME/libs:$LD_LIBRARY_PATH
export SDL_VIDEODRIVER=mmiyoo
export SDL_AUDIODRIVER=mmiyoo

cd $mydir

vol=`/customer/app/jsonval vol`
vol=`expr 41 + 63 \* $vol \/ 20`
/customer/app/tinymix set 6 $vol

./OpenBOR "$1"