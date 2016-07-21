#!/bin/ash

. /tmp/flags

W_ROOT=/dev/mmcblk0
export W_ROOTDEV=${W_ROOT}p1
export W_MODDEV=${W_ROOT}p2

if [ -z "$W_UPDATESERVER" ]; then
	export W_UPDATESERVER=http://wallaby.freshx.de/wally/ose/update
fi
if [ -z "$W_UPDATEURL" ]; then
	export W_UPDATEURL=$W_UPDATESERVER/$W_PLATFORM/$W_ARCH
fi

XPOS=$(((W_WIDTH-1240)/2))
YPOS=$((W_HEIGHT-200))
wally settext2 $XPOS $YPOS Firmware recovery

ERR=1
for i in `seq 1 180`; do
     if [ ! -z "`ip a|grep "inet "`" ]; then
          ERR=0
          break
     fi
     log "Waiting for an IP Address (since $i seconds)"
     sleep 1
done

if [ $ERR -gt 0 ]; then
    log Could not get a network connection. Recovery process stopped.
    sleep 7200
    reboot
fi

cd /tmp
test -f wally-version && rm wally-version
wget --no-check-certificate $W_UPDATEURL/wally-version
if [ $? -gt 0 ]; then
    log Could not reach Wally Update Server at $W_UPDATEURL. Recovery process stopped.
    sleep 7200
    reboot
fi

VERSION=`cat wally-version | sed "s/VERSION=//" | tr -d "\n"`

#       Start a killing session of all shell based processes (besides ourselves)
ps |grep -v grep|egrep "(\.sh|node|phantom)" |grep -v $$ | grep -v unionfs | awk '{print $1}' | xargs kill -9

ERR=1
for i in `seq 1 180`; do
    cat /proc/mounts | egrep "(squash|union|fat)" | awk '{print $2}' | xargs umount
    for j in `losetup  | awk -F: '{print $1}'`; do losetup -d $j; done
    sync
    if [ "$i" -gt 6 ]; then
        killall -9 unionfs
        killall -9 funionfs
    fi
    if [ -z "`cat /proc/mounts |grep fat`" ]; then
          ERR=0
          break
    fi
    log "Waiting for SDCard to get ready (since $i seconds)"
    sleep 1
done

if [ $ERR -gt 0 ]; then
    log SDCard still busy. Recovery process stopped.
    sleep 7200
    reboot
fi
ERR=1

log Partition and Format SDCard 

# Delete Partitions
log "Partition and Format SDCard (Step 1/4)"
echo -e "d\n1\nw" | fdisk $W_ROOT
echo -e "d\n2\nw" | fdisk $W_ROOT
echo -e "d\n3\nw" | fdisk $W_ROOT
echo -e "d\n4\nw" | fdisk $W_ROOT

# Create P1 128MB type 6 (FAT32)
log "Partition and Format SDCard (Step 2/4)"
echo -e "n\np\n1\n1\n+128M\nt\nb\nw" | fdisk $W_ROOT

# Create P2 1024M type b (FAT32)
echo -e "n\np\n2\n\n+1024M\nw" | fdisk $W_ROOT
echo -e "t\n2\nb\nw" | fdisk $W_ROOT
log "Partition and Format SDCard (Step 3/4)"
mkfs.vfat -n BOOT $W_ROOTDEV
log "Partition and Format SDCard (Step 4/4)"
mkfs.vfat -n MODULES $W_MODDEV

mount -o rw $W_ROOTDEV /mnt/local
cd /mnt/local

log "Restoring firmware : BootLoader to R$VERSION"
wget --no-check-certificate $W_UPDATEURL/boot.tgz
tar xfz boot.tgz
rm boot.tgz

log "Restoring firmware : WallyCore to R$VERSION"
wget --no-check-certificate $W_UPDATEURL/initrd.gz
if [ $? -gt 0 ]; then
        ERR=$(( ERR+1 ))
        log "Restoring firmware : core failed!"
        cd /
        umount /mnt/local
        sleep 7200
        reboot
fi

echo VERSION=$VERSION >wally-version

cd /
umount /mnt/local
sync

log "Restoring firmware : Modules to R$VERSION"
export RECOVERY_MODULES="addons.sqsh fonts.sqsh phantomjs.sqsh  wlan.sqsh nodejs.sqsh wallytv.sqsh"
/root/updateMods.sh && reboot
