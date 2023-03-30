#!/bin/sh
cd /home/bittboy/git/invoker/invoker
make clean
make PLATFORM=RG-350
cp /home/bittboy/git/invoker/invoker/invoker.dge /home/bittboy/git/simplemenu/simplemenu/output/invoker.dge
cd /home/bittboy/git/simplemenu/simplemenu/
make clean
make PLATFORM=RG-350
cd output
./make_opk.sh
while true; do
    read -p "Transfer?" yn
    case $yn in
        [Yy]* ) scp SimpleMenu.opk root@10.1.1.2:/media/data/apps/SimpleMenu-RG-350.opk; break;;
        [Nn]* ) exit;;
        * ) echo "Please answer yes or no.";;
    esac
done
