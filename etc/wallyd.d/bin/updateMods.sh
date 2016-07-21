#!/bin/ash

. /tmp/flags

if [ -z "$W_MODDEV" ]; then
	log "W_MODDEV not set. Can not continue."
	exit 1
fi
if [ -z "$W_ROOTDEV" ]; then
	log "W_ROOTDEV not set. Can not continue."
	exit 1
fi
if [ -z "$W_UPDATESERVER" ]; then
	W_UPDATESERVER=http://wallaby.freshx.de/wally/ose/update
fi
if [ -z "$W_UPDATEURL" ]; then
	W_UPDATEURL=$W_UPDATESERVER/$W_PLATFORM/$W_ARCH
	log Update URL is not set. Using default.
fi

ERR=0

ifconfig eth0 mtu 1480

kill `ps | grep phantomjs |grep -v union| grep -v grep | awk '{print $1}'`
kill `ps | grep node |grep -v union| grep -v grep | awk '{print $1}'`
kill `ps | grep wally | grep -v wallyd |grep -v union| grep -v grep | awk '{print $1}'`
sleep 2

log "Starting update process from $W_UPDATEURL."

COUNT=1
if [ -z `cat /proc/mounts|grep fat` ]; then
    mount -t vfat -o rw $W_MODDEV /modules
else
    mount -o remount,rw $W_MODDEV
fi
cd /modules
test -d update || mkdir update

if [ ! -z "$RECOVERY_MODULES" ]; then
    MODULES=$RECOVERY_MODULES
else
    MODULES=`ls *.sqsh`
fi

MODCOUNT=`echo $MODULES | wc -w`

for i in `echo $MODULES`; do
	log "Updating firmware module : `echo $i | sed "s/.sqsh//"` ($COUNT of $MODCOUNT)"
	wget --no-check-certificate $W_UPDATEURL/$i -P update
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
    log Failed to update modules, reverting process
    exit 1
fi
for i in `echo $MODULES`; do
    test -e $i && rm $i
    mv update/$i $i
done

rmdir update

cd /
mount -o remount,ro $W_MODDEV
log Update done. $ERR errors occured, $COUNT modules updated.
