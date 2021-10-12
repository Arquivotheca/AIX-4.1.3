# @(#)54  1.20.2.22  src/bos/usr/lib/boot/rs6k.disk.proto.S, bosboot, bos41J, 9510B 3/8/95 09:41:29
#
# COMPONENT_NAME: (BOSBOOT) Base Operating System Boot
#
# FUNCTIONS: disk.proto.S
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
/bootrec 8704 0
d--- 777 0 0
bin     l--- 777 0 0 /usr/bin
dev     d--- 755 0 0
	audit     c--t 440 0 0 8 0
	bus0      c--- 660 0 0 3 0
	console   c--- 622 0 0 4 0
	error     c--- 622 0 0 6 0
	errorctl  c--- 600 0 0 6 1
	kmem      c--- 440 0 0 2 1
	mem       c--- 440 0 0 2 0
	null      c--- 666 0 0 2 2
	nvram     c--t 660 0 0 3 0
	ram0      b--- 600 0 0 0 0
	ram1      b--- 600 0 0 0 1
	sysdump   c--- 600 0 0 7 0
	sysdumpctl c--- 600 0 0 7 1
	sysdumpfile c--- 600 0 0 7 3
	sysdumpnull c--- 600 0 0 7 2
	tty       c--- 666 0 0 1 0
	zero      c--- 666 0 0 2 3
	$
etc     d--- 777 0 0
	drivers     l--- 777 0 0 /usr/lib/drivers
	filesystems ---- 777 0 0 /etc/filesystems
	init	---- 777 0 0 /usr/lib/boot/ssh
	methods     l--- 777 0 0 /usr/lib/methods
	microcode   l--- 777 0 0 /usr/lib/microcode
	objrepos d--- 777 0 0
		Config_Rules ---- 777 0 0 /tmp/boot_IBM/Config_Rules
		CuAt    ---- 777 0 0 /tmp/boot_IBM/CuAt
		CuAt.vc ---- 777 0 0 /tmp/boot_IBM/CuAt.vc
		CuDep   ---- 777 0 0 /tmp/boot_IBM/CuDep
		CuDv    ---- 777 0 0 /tmp/boot_IBM/CuDv
		CuDvDr  ---- 777 0 0 /tmp/boot_IBM/CuDvDr
		CuVPD   ---- 777 0 0 /tmp/boot_IBM/CuVPD
		PdAt    ---- 777 0 0 /tmp/boot_IBM/PdAt
		PdAt.vc ---- 777 0 0 /tmp/boot_IBM/PdAt.vc
		PdCn    ---- 777 0 0 /tmp/boot_IBM/PdCn
		PdDv    ---- 777 0 0 /tmp/boot_IBM/PdDv
		PdDv.vc ---- 777 0 0 /tmp/boot_IBM/PdDv.vc
		SWservAt l--- 777 0 0 /mnt/etc/objrepos/SWservAt
		SWservAt.vc l--- 777 0 0 /mnt/etc/objrepos/SWservAt.vc
		$
	restbase    ---- 777 0 0 /usr/lib/boot/restbase
	vfs	 ---- 777 0 0 /etc/vfs
	vg      d--- 775 0 0
		$
	$
lib     d--- 777 0 0
	$
mnt     d--- 777 0 0
	$
sbin    d--- 777 0 0
	helpers d--- 777 0 0
		v3fshelper ---- 777 0 0 /sbin/helpers/v3fshelper
		$
	rc.boot ---- 777 0 0 /sbin/rc.boot
	$
tmp     d--- 777 0 0
	$
usr     d--- 777 0 0
	bin     d--- 777 0 0
		bsh  l--- 555 0 0 /usr/bin/ksh
		ksh  ---- 555 0 0 /usr/bin/ksh
		ln      ---- 777 0 0 /usr/bin/ln
		sh  l--- 555 0 0 /usr/bin/ksh
		uncompress  ---- 777 0 0 /usr/bin/uncompress
		$
	lib     d--- 777 0 0
		drivers d--- 777 0 0
			hd_pin     ---- 777 0 0 /usr/lib/drivers/hd_pin
			hd_pin_bot ---- 777 0 0 /usr/lib/drivers/hd_pin_bot
			$
		libc.a    ---- 555 0 0 /usr/ccs/lib/libc.a.min
		libcfg.a  ---- 555 0 0 /tmp/libcfg.a
		liblvm.a  ---- 555 0 0 /tmp/liblvm.a
		libodm.a  ---- 555 0 0 /tmp/libodm.a
		libs.a    ---- 555 0 0 /usr/lib/libs.a.min
		methods d--- 777 0 0
			cfglvdd    ---- 777 0 0 /usr/lib/methods/cfglvdd
			cfgsys  ---- 777 0 0 /usr/lib/methods/cfgsys
			define     ---- 777 0 0 /usr/lib/methods/define
			deflvm     ---- 777 0 0 /usr/lib/methods/deflvm
			showled	---- 777 0 0 /usr/lib/methods/showled
			$
		$
	sbin    d--- 777 0 0
		bootinfo	---- 777 0 0 /usr/sbin/bootinfo
		cfgmgr      ---- 777 0 0 /usr/sbin/cfgmgr
		chramfs  ---- 777 0 0 /usr/lib/boot/chramfs
		fsck	---- 777 0 0 /usr/sbin/fsck
		ipl_varyon      ---- 777 0 0 /usr/sbin/ipl_varyon
		logform	 ---- 777 0 0 /usr/sbin/logform
		logredo	 ---- 777 0 0 /usr/sbin/logredo
		mknod       ---- 777 0 0 /usr/sbin/mknod
		mount       ---- 777 0 0 /usr/sbin/mount
		swapon      ---- 777 0 0 /usr/sbin/swapon
		$
	$
var     d--- 777 0 0
	$
$
