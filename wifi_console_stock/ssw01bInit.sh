#!/bin/sh

/config/riu_w e 30 11
/config/riu_w 103c 8 00
sleep 0.01 
/config/riu_w 103c 8 10
#mkdir -p /etc/
#touch /etc/hosts
touch /appconfigs/hosts
mkdir -p /tmp/wifi/run
chmod 777 /tmp/wifi/run
mkdir -p /appconfigs/misc/wifi/
mkdir -p /var/wifi/misc/
mkdir -p /var/lib/misc/
mkdir -p /var/run/hostapd/
insmod /config/wifi/ssw101b_wifi_HT40_usb.ko
wlan0=`ifconfig -a | grep wlan0`
trial=0
while [ -z "$wlan0" ] && [ $trial -le 20 ]
do 
    sleep 0.2
    trial=$(($trial + 1 ))
    wlan0=`ifconfig -a | grep wlan0`
done
if [ $trial -gt 20 ];then
    echo wlan0 not found
    exit -1
fi
echo LOG_WARN=OFF > /sys/module/ssw101b_wifi_usb/Sstarfs/Sstar_printk_mask
echo LOG_INIT=OFF > /sys/module/ssw101b_wifi_usb/Sstarfs/Sstar_printk_mask
echo LOG_EXIT=OFF > /sys/module/ssw101b_wifi_usb/Sstarfs/Sstar_printk_mask
echo LOG_SCAN=OFF > /sys/module/ssw101b_wifi_usb/Sstarfs/Sstar_printk_mask
echo LOG_LMAC=OFF > /sys/module/ssw101b_wifi_usb/Sstarfs/Sstar_printk_mask
echo LOG_PM=OFF > /sys/module/ssw101b_wifi_usb/Sstarfs/Sstar_printk_mask
exit 0







