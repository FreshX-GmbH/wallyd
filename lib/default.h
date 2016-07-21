#ifndef WALLY_DEFAULTS
#define WALLY_DEFAULTS

#define FIFO "/tmp/wally.sock"
#define FLAGFILE "/tmp/flags"

#define KERNEL_DEFAULT_CONFIG "dwc_otg.lpm_enable=0 consoleblank=0 console=ttyAMA0,115200 kgdboc=ttyAMA0,115200 W_ROOTDEV=/dev/mmcblk0p1 W_MODDEV=/dev/mmcblk0p2 vt.global_cursor_default=0"

#define DEFAULT_CMDLINE_TXT "/tmp/cmdline.txt.new"
#define DEFAULT_CONFIG_TXT "/tmp/cmdline.txt.new"
#define DEFAULT_UBOOT_ENV "/tmp/uboot.env"

#define BIN_UPDATEFW "/root/updateFW.sh"
#define BIN_REBOOT "/sbin/reboot"
#define BIN_PERSIST "/root/persist.sh"

#define ETC_CONFIG      "/etc/wallyd.conf"
#define ETC_CONFIG_BAK  "etc/wallyd.conf"
#define ETC_FLAGS       "/tmp/flags"
#define ETC_FLAGS_BAK   "etc/flags"

#define DEFAULT_MAC    "00:00:00:00:08:15"

#define MAXFONTS       16
#define MAXTEXTURES    8

#endif
