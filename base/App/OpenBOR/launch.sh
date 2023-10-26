#!/bin/sh
HOME=`dirname "$0"`

setvolume () {
  vol=$(/customer/app/jsonval vol)
  volume=$((($vol*3)+40))
  /customer/app/tinymix set 6 $volume
}

export PATH=$HOME
export LD_LIBRARY_PATH=$HOME/libs:$LD_LIBRARY_PATH
export SDL_VIDEODRIVER=mmiyoo
export SDL_AUDIODRIVER=mmiyoo

cd $mydir

setvolume &

./OpenBOR "$1"