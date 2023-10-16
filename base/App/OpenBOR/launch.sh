#!/bin/sh
HOME=`dirname "$0"`

export PATH=$HOME
export LD_LIBRARY_PATH=$HOME/libs:$LD_LIBRARY_PATH
export SDL_VIDEODRIVER=mmiyoo
export SDL_AUDIODRIVER=mmiyoo

cd $mydir
./OpenBOR "$1"