# @(#)71  1.23  src/bos/usr/lib/boot/rs6k.tape.proto.S, bosboot, bos411, 9428A410j 6/21/94 16:52:24
#
#   COMPONENT_NAME: BOSBOOT
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1989,1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
/bootrec 13312 0
d--- 755 0 0
bin     l--- 777 0 0 /usr/bin
dev     d--- 755 0 0
	bus0	  c--- 640 0 0 3 0
	console   c--- 622 0 0 4 0
	error	  c--- 622 0 0 6 0
	errorctl  c--- 600 0 0 6 1
	kmem	  c--- 440 0 0 2 1
	mem	  c--- 440 0 0 2 0
	null	  c--- 666 0 0 2 2
	nvram	  c--t 640 0 0 3 0
	ram0	  b--- 600 0 0 0 0
	ram1	  b--- 600 0 0 0 1
	sysdump   c--- 600 0 0 7 0
	sysdumpctl  c--- 600 0 0 7 1
	sysdumpfile c--- 600 0 0 7 3
	sysdumpnull c--- 600 0 0 7 2
	tty	  c--- 666 0 0 1 0
	$
etc	d--- 755 0 0
	drivers	l--- 777 0 0 /usr/lib/drivers
	init       ---- 777 0 0 /usr/lib/boot/ssh
	methods l--- 777 0 0 /usr/lib/methods
	microcode l--- 777 0 0 /usr/lib/microcode
	objrepos d--- 777 0 0
		Config_Rules	---- 777 0 0 /tmp/tapeboot_IBM/Config_Rules
		CuAt	---- 777 0 0 /tmp/tapeboot_IBM/CuAt
		CuAt.vc ---- 777 0 0 /tmp/tapeboot_IBM/CuAt.vc
		CuDep	---- 777 0 0 /tmp/tapeboot_IBM/CuDep
		CuDv	---- 777 0 0 /tmp/tapeboot_IBM/CuDv
		CuDvDr	---- 777 0 0 /tmp/tapeboot_IBM/CuDvDr
		CuVPD	---- 777 0 0 /tmp/tapeboot_IBM/CuVPD
		PdAt	---- 777 0 0 /tmp/tapeboot_IBM/PdAt
		PdAt.vc ---- 777 0 0 /tmp/tapeboot_IBM/PdAt.vc
		PdCn	---- 777 0 0 /tmp/tapeboot_IBM/PdCn
		PdDv	---- 777 0 0 /tmp/tapeboot_IBM/PdDv
		PdDv.vc ---- 777 0 0 /tmp/tapeboot_IBM/PdDv.vc
		$
	vg	d--- 777 0 0
		$
	$
lib	l--- 755 0 0 /usr/lib
mnt	d--- 755 0 0
	$
sbin	d--- 755 0 0
	rc.boot   ---- 777 0 0 /sbin/rc.boot
	$
tmp	d--- 777 0 0
	$
usr	d--- 755 0 0
	bin	d--- 755 0 0
		bsh  l--- 555 0 0 /usr/bin/ksh
		ksh  ---- 555 0 0 /usr/bin/ksh
		mkdir  ---- 555 0 0 /usr/bin/mkdir
		odmadd  ---- 777 0 0 /usr/bin/odmadd
		odmdelete  ---- 777 0 0 /usr/bin/odmdelete
		odmget  ---- 777 0 0 /usr/bin/odmget
		sh  l--- 555 0 0 /usr/bin/ksh
		tctl  ---- 777 0 0 /usr/bin/tctl
		$
	lib	d--- 755 0 0
		drivers	d--- 777 0 0
			hd_pin	---- 777 0 0 /usr/lib/drivers/hd_pin
			hd_pin_bot  ---- 777 0 0 /usr/lib/drivers/hd_pin_bot
			ldterm  ---- 777 0 0 /usr/lib/drivers/ldterm
			tioc     ---- 777 0 0 /usr/lib/drivers/tioc
			pse	d--- 755 0 0
				pse  ---- 777 0 0 /usr/lib/drivers/pse/pse
				$
			$
		libc.a	---- 555 0 0 /usr/ccs/lib/libc.a.min
		libcfg.a ---- 555 0 0 /tmp/libcfg.a
		libodm.a ---- 555 0 0 /tmp/libodm.a
		libs.a   ---- 555 0 0 /usr/lib/libs.a.min
		methods d--- 777 0 0
			cfglvdd  ---- 777 0 0 /usr/lib/methods/cfglvdd
			chggen  ---- 777 0 0 /usr/lib/methods/chggen
			cfgsys  ---- 777 0 0 /usr/lib/methods/cfgsys
			define   ---- 777 0 0 /usr/lib/methods/define
			deflvm  ---- 777 0 0 /usr/lib/methods/deflvm
			instdbcln  ---- 777 0 0 /usr/lib/methods/instdbcln
			showled	---- 777 0 0 /usr/lib/methods/showled
			ucfgdevice  ---- 777 0 0 /usr/lib/methods/ucfgdevice
			$
		microcode d--- 777 0 0
			$
		nls	d--- 755 0 0
			loc	d--- 755 0 0
				C.lftkeymap  ---- 777 0 0 /usr/lib/nls/loc/C.lftkeymap
				$
			$
		$
	lpp	d--- 755 0 0
		bosinst	d--- 755 0 0
			bi_main  ---- 777 0 0 /usr/lpp/bosinst/bi_main
			$
		fonts	d--- 755 0 0
			$
		$
	sbin	d--- 755 0 0
		bootinfo   ---- 777 0 0 /usr/sbin/bootinfo
		cfgmgr     ---- 777 0 0 /usr/sbin/cfgmgr
		lsdev      ---- 777 0 0 /usr/sbin/lsdev
		restbyname ---- 777 0 0 /usr/sbin/restbyname
		strload    ---- 777 0 0 /usr/sbin/strload
		$
	$
$
