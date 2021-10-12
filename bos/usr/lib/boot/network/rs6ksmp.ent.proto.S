# @(#)19  1.3  src/bos/usr/lib/boot/network/rs6ksmp.ent.proto.S, bosboot, bos411, 9440A411c 10/4/94 13:34:22
#
#   COMPONENT_NAME: BOSBOOT
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
/bootrec 8192 0
d--- 755 0 0
bin	l--- 755 0 0 /usr/bin
dev	d--- 755 0 0
	audit     c--t 440 0 0 8 0
	bus0	  c--- 640 0 0 3 0
	console   c--- 622 0 0 4 0
	error     c--- 622 0 0 6 0
	errorctl  c--- 600 0 0 6 1
	kmem	  c--- 440 0 0 2 1
	mem	  c--- 440 0 0 2 0
	null	  c--- 666 0 0 2 2
	nvram	  c--t 640 0 0 3 0
	ram0	  b--- 600 0 0 0 0
	ram1	  b--- 600 0 0 0 1
	sysdump   c--- 600 0 0 7 0
	sysdumpctl c--- 600 0 0 7 1
	sysdumpfile c--- 600 0 0 7 3
	sysdumpnull c--- 600 0 0 7 2
	tty	  c--- 666 0 0 1 0
	zero      c--- 666 0 0 2 3
	$
etc	d--- 755 0 0
	drivers	l--- 777 0 0 /usr/lib/drivers
	init	---- 555 0 0 /usr/lib/boot/ssh
	methods l--- 777 0 0 /usr/lib/methods
	microcode l--- 777 0 0 /usr/lib/microcode
	niminfo l--- 755 0 0 /SPOT/niminfo
	objrepos d--- 777 0 0
		Config_Rules	---- 777 0 0 /tmp/netboot_IBM/Config_Rules
		CuDep	---- 777 0 0 /tmp/netboot_IBM/CuDep
		CuDv	---- 777 0 0 /tmp/netboot_IBM/CuDv
		CuDvDr	---- 777 0 0 /tmp/netboot_IBM/CuDvDr
		CuAt	---- 777 0 0 /tmp/netboot_IBM/CuAt
		CuAt.vc ---- 777 0 0 /tmp/netboot_IBM/CuAt.vc
		CuVPD	---- 777 0 0 /tmp/netboot_IBM/CuVPD
		PdAt	---- 777 0 0 /tmp/netboot_IBM/PdAt
		PdAt.vc ---- 777 0 0 /tmp/netboot_IBM/PdAt.vc
		PdCn	---- 777 0 0 /tmp/netboot_IBM/PdCn
		PdDv	---- 777 0 0 /tmp/netboot_IBM/PdDv
		PdDv.vc ---- 777 0 0 /tmp/netboot_IBM/PdDv.vc
		$
	services	---- 555 0 0 /etc/services
	vfs		---- 555 0 0 /etc/vfs
	vg	d--- 777 0 0
		$
	$
lib	l--- 755 0 0 /usr/lib
mnt	d--- 755 0 0
	$
SPOT	d--- 755 0 0
	usr	d--- 755 0 0
		$
	$
sbin	d--- 755 0 0
	helpers	d--- 777 0 0
		nfsmnthelp   ---- 777 0 0 /sbin/helpers/nfsmnthelp
		v3fshelper   ---- 777 0 0 /sbin/helpers/v3fshelper
		$
	rc.boot ---- 777 0 0 /sbin/rc.boot
	$
tmp	d--- 777 0 0
	$
usr	d--- 755 0 0
	bin	d--- 755 0 0
		bsh	l--- 555 0 0 /usr/bin/ksh
		cp	---- 555 0 0 /usr/bin/cp
		ksh	---- 555 0 0 /usr/bin/ksh
		ln	---- 555 0 0 /usr/bin/ln
		sh	l--- 555 0 0 /usr/bin/ksh
		tftp	---- 777 0 0 /usr/bin/tftp
		$
	lib	d--- 777 0 0
		drivers	d--- 755 0 0
			if_en	---- 777 0 0 /usr/lib/drivers/if_en
			netinet	---- 777 0 0 /usr/lib/drivers/netinet
			nfs_clnt.ext	---- 777 0 0 /usr/lib/drivers/nfs_clnt.ext
			nfs_clnt.load	---- 777 0 0 /usr/lib/drivers/nfs_clnt.load
			nfs_kdes.ext	---- 777 0 0 /usr/lib/drivers/nfs_kdes.ext
			nfs_krpc.ext	---- 777 0 0 /usr/lib/drivers/nfs_krpc.ext
			$
		libbsd.a ---- 555 0 0 /tmp/libbsd.a
		libc.a	 ---- 555 0 0 /usr/ccs/lib/libc.a.min
		libcfg.a ---- 555 0 0 /tmp/libcfg.a
		libodm.a ---- 555 0 0 /tmp/libodm.a
		librpcsvc.a ---- 555 0 0 /tmp/librpcsvc.a
		libs.a   ---- 555 0 0 /usr/lib/libs.a.min
		libsrc.a ---- 555 0 0 /tmp/libsrc.a
		methods d--- 777 0 0
			cfgsys_p   ---- 777 0 0 /usr/lib/methods/cfgsys_p
			define   ---- 777 0 0 /usr/lib/methods/define
			showled	---- 555 0 0 /usr/lib/methods/showled
			ucfgcommo   ---- 777 0 0 /usr/lib/methods/ucfgcommo
			undefine   ---- 777 0 0 /usr/lib/methods/undefine
			$
		microcode d--- 777 0 0
			$
		netsvc	d--- 777 0 0
			liblocal ---- 555 0 0 /usr/lib/netsvc/liblocal
			$
		$
	sbin	d--- 777 0 0
		bootinfo	---- 777 0 0 /usr/sbin/bootinfo
		cfgmgr	---- 777 0 0 /usr/sbin/cfgmgr
		ifconfig	---- 777 0 0 /usr/sbin/ifconfig
		mount		---- 777 0 0 /usr/sbin/mount
		route		---- 777 0 0 /usr/sbin/route
		$
	$
var	d--- 777 0 0
	adm	d--- 777 0 0
		nim	d--- 777 0 0
			$
		ras	d--- 777 0 0
			$
		$
	$
$
