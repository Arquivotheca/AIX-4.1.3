# @(#)18        1.1  src/bldenv/proto_con/dskt.proto.S, bldprocess, bos412, GOLDA411a 8/7/92 18:24:23
#
# COMPONENT_NAME: (BOSBOOT) Base Operating System Boot
#
# FUNCTIONS: dskt.proto.S
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#                                                                   
bootrec 9000 0
d--- 755 0 0
bin	d--- 755 0 0
	sh      ---- 555 0 0 /sbin/bsh
	$
dev     d--- 755 0 0
	ram0	  b--- 600 0 0 0 0
	ram1	  b--- 600 0 0 0 1
	tty	  c--- 666 0 0 1 0
	mem	  c--- 440 0 0 2 0
	kmem 	  c--- 440 0 0 2 1
	null	  c--- 666 0 0 2 2
	bus0	  c--- 640 0 0 3 8
	nvram	  c--t 640 0 0 3 0
        console   c--- 622 0 0 4 0
	error	  c--- 622 0 0 6 0
	errorctl  c--- 600 0 0 6 1
	sysdump   c--- 600 0 0 7 0
	sysdumpctl  c--- 600 0 0 7 1
	sysdumpnull c--- 600 0 0 7 2
	sysdumpfile c--- 600 0 0 7 3
	$  
etc	d--- 755 0 0
	init       ---- 555 0 0 /usr/lib/boot/ssh
	showled    ---- 555 0 0 /sbin/showled
	restbyname ---- 555 0 0 /sbin/restbyname
	cfgmgr     ---- 555 0 0 /sbin/cfgmgr
	lsdev      ---- 555 0 0 /sbin/lsdev
	methods d--- 777 0 0
		cfgbus   ---- 777 0 0 /sbin/cfgbus
		cfgfda   ---- 777 0 0 /sbin/cfgfda
		cfgfdd   ---- 777 0 0 /sbin/cfgfdd
		cfgsio   ---- 777 0 0 /sbin/cfgsio
		cfgsys   ---- 777 0 0 /sbin/cfgsys
		define   ---- 777 0 0 /sbin/define
		defsys   ---- 777 0 0 /sbin/defsys
		instdbcln   ---- 777 0 0 /sbin/instdbcln
		$
	objrepos d--- 777 0 0
		Config_Rules	---- 777 0 0 /tmp/boot_IBM/Config_Rules
		CuDep	---- 777 0 0 /tmp/boot_IBM/CuDep
		CuDv	---- 777 0 0 /tmp/boot_IBM/CuDv
		CuDvDr	---- 777 0 0 /tmp/boot_IBM/CuDvDr
		CuAt	---- 777 0 0 /tmp/boot_IBM/CuAt
		CuVPD	---- 777 0 0 /tmp/boot_IBM/CuVPD
		PdAt	---- 777 0 0 /tmp/boot_IBM/PdAt
		PdCn	---- 777 0 0 /tmp/boot_IBM/PdCn
		PdDv	---- 777 0 0 /tmp/boot_IBM/PdDv
		$
	stanzas	d--- 777 0 0
		$
	drivers	d--- 777 0 0
		fd       ---- 777 0 0 /usr/lib/drivers/fd
		$
	microcode d--- 777 0 0
		$
	vg	 d--- 775 0 0
		$
	$
tmp	d--- 777 0 0
	$
mnt	d--- 755 0 0
	$
sbin	d--- 755 0 0
	rc.boot ---- 555 0 0 /sbin/rc.boot
	$
usr	d--- 755 0 0
	bin	d--- 755 0 0
		odmadd    ---- 555 0 0 /usr/sbin/odmadd
		odmget    ---- 555 0 0 /usr/sbin/odmget
		odmdelete ---- 555 0 0 /usr/sbin/odmdelete
		$
	sbin	d--- 755 0 0
		bootinfo   ---- 777 0 0 /usr/sbin/bootinfo
		$
	lib	d--- 755 0 0
		nls	d--- 755 0 0
			loc	d--- 755 0 0
				$
			$
		$
	lbin	d--- 755 0 0
		tty	d--- 777 0 0
			$
		$
	lpp	d--- 755 0 0
		fonts	d--- 755 0 0
			$
		$
	$
lib	d--- 755 0 0
	libc.a	 ---- 555 0 0 /usr/ccs/lib/libc.a.min
	libs.a   ---- 555 0 0 /tmp/libs.a
	libodm.a ---- 555 0 0 /tmp/libodm.a
	libcfg.a ---- 555 0 0 /tmp/libcfg.a
	$
$
