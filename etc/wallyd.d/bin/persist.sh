#!/bin/sh

echo Persisting configs to SDCard
test -d /mnt/tmp || mkdir /mnt/tmp
mount -t vfat /dev/mmcblk0p1 /mnt/tmp
cp /tmp/config.txt.new /mnt/tmp/config.txt
cp /tmp/cmdline.txt.new /mnt/tmp/cmdline.txt
umount /mnt/tmp
sync
sleep 2
reboot
