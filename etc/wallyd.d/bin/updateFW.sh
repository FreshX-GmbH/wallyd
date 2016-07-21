#!/bin/ash

. /tmp/flags

mv /tmp/wally.sock /tmp/wally.sock.update

wallyFWU setpngscaled /root/images/wally1920x1080.png 
XPOS=$(((W_WIDTH-1120)/2))
YPOS=$((W_HEIGHT-200))
wallyFWU settext2 $XPOS $YPOS Firmware update

logfwu Updating firmware...

if [ -f /etc/wally-devicetype ]; then
	. /etc/wally-devicetype
else
	echo "WARNING : /etc/wally-devicetype does not exist. Trying to autodetermine"
	W_ARCH=`uname -p`
	W_PLATFORM=default	
fi

if [ -z "$W_MODDEV" ]; then
	logfwu "W_MODDEV not set. Can not continue."
	exit 1
fi
if [ -z "$W_ROOTDEV" ]; then
	logfwu "W_ROOTDEV not set. Can not continue."
	exit 1
fi
if [ -z "$W_US" ]; then
	W_UPDATESERVER=http://wallaby.freshx.de/wally/ose/update
else
	W_UPDATESERVER=$W_US
fi
if [ -z "$W_UPDATEURL" ]; then
	W_UPDATEURL=$W_UPDATESERVER/$W_PLATFORM/$W_ARCH
	logfwu Update URL is not set. Using default.
fi

ERR=0

ifconfig eth0 mtu 1480

kill `ps | grep phantomjs |grep -v union| grep -v grep | awk '{print $1}'`
kill `ps | grep node |grep -v union| grep -v grep | awk '{print $1}'`
kill `ps | grep wally | grep -v wallyd |grep -v union| grep -v grep | awk '{print $1}'`
sleep 2

logfwu "Starting update process from $W_UPDATEURL."
logfwu "Updating firmware : core"
mount -o rw $W_ROOTDEV /mnt/local
cd /mnt/local

mv initrd.gz initrd.gz.bak
wget --no-check-certificate $W_UPDATEURL/initrd.gz
if [ $? -gt 0 ]; then
	ERR=$(( ERR+1 ))
	mv initrd.gz.bak initrd.gz
	logfwu "Updating firmware : core failed!"
	cd /
	umount /mnt/local
	exit 1
fi
mv wally-version wally-version.old
wget --no-check-certificate $W_UPDATEURL/wally-version
if [ $? -gt 0 ]; then
	rm wally-version.old
else 
	mv wally-version.old wally-version
fi
cd /
umount /mnt/local

if [ "$W_FWONLY" ]; then
	exit 0
fi

COUNT=0
mount -o remount,rw $W_MODDEV
cd /modules
test -d update || mkdir update
MODULES=`ls *.sqsh`
if [ ! -z "$RECOVERY_MODULES" ]; then
    MODULES=$RECOVERY_MODULES
fi
for i in `echo $MODULES`; do
	#mv $i $i.bak
	wget --no-check-certificate $W_UPDATEURL/$i -P update
	logfwu "Updating firmware module : `echo $i | sed "s/.sqsh//"`"
	#rm $i.bak
	sync
	if [ $? -gt 0 ]; then
		ERR=$(( ERR+1 ))
	fi
	COUNT=$((COUNT+1))
done
if [ $ERR -gt 0 ]; then
    rm -rf update
    sync
    cd /
    mount -o remount,ro $W_MODDEV
    logfwu Failed to update modules, reverting process
    exit 1
fi
for i in `ls *.sqsh`; do
    rm $i
    mv update/$i $i
done

rmdir update

cd /
mount -o remount,ro $W_MODDEV
logfwu Update done. $ERR errors occured, $COUNT modules updated.

if [ $ERR -eq 0 ]; then
	reboot
fi
