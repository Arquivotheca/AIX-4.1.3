# @(#)77	1.13  src/bos/diag/boot/cdrom.proto.S, diagboot, bos41J, 9515A_all 4/5/95 10:11:51
#
#   COMPONENT_NAME: DIAGBOOT
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
/bootrec 8000 0
d--- 755 0 0
bin     l--- 777 0 0 /usr/bin
dev     d--- 755 0 0
	bus0	  c--- 640 0 0 3 0
	console   c--- 622 0 0 4 0
	error	  c--- 622 0 0 6 0
	errorctl  c--- 600 0 0 6 1
	kmem 	  c--- 440 0 0 2 1
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
	lpp	l--- 777 0 0 /usr/lpp
	methods l--- 777 0 0 /usr/lib/methods
	microcode l--- 777 0 0 /usr/lib/microcode
	drivers	l--- 777 0 0 /usr/lib/drivers
	init       ---- 777 0 0 /usr/lib/boot/ssh
	cfgmgr     ---- 777 0 0 /usr/lpp/diagnostics/bin/cfgmgr
	flattendb  ---- 555 0 0 /tmp/flattendb
	rmfiles	   ---- 555 0 0 /tmp/rmfiles
	chkdskt	   ---- 555 0 0 /usr/lpp/diagnostics/bin/chkdskt
	eject	   ---- 555 0 0 /usr/lpp/diagnostics/bin/eject
	graphics1.dep ---- 555 0 0 /tmp/graphics1.dep
	graphics2.dep ---- 555 0 0 /tmp/graphics2.dep
	linkedfiles.dep ---- 555 0 0 /tmp/linkedfiles.dep
        vfs     ---- 555 0 0 /etc/vfs
	profile ---- 555 0 0 /etc/profile
	passwd  ---- 555 0 0 /etc/passwd
	objrepos d--- 777 0 0
		Config_Rules	---- 777 0 0 /tmp/Config_Rules
		CuAt	---- 777 0 0 /tmp/CuAt
		CuAt.vc	---- 777 0 0 /tmp/CuAt.vc
		CuDep	---- 777 0 0 /tmp/CuDep
		CuDv	---- 777 0 0 /tmp/CuDv
		CuDvDr	---- 777 0 0 /tmp/CuDvDr
		CuVPD	---- 777 0 0 /tmp/CuVPD
		PdAt	---- 777 0 0 /tmp/PdAt
		PdAt.vc	---- 777 0 0 /tmp/PdAt.vc
		PdCn	---- 777 0 0 /tmp/PdCn
		PdDv	---- 777 0 0 /tmp/PdDv
		PdDv.vc	---- 777 0 0 /tmp/PdDv.vc
		PDiagDev ---- 777 0 0 /tmp/PDiagDev
		PDiagDev.vc ---- 777 0 0 /tmp/PDiagDev.vc
		PDiagAtt ---- 777 0 0 /tmp/PDiagAtt
		PDiagAtt.vc ---- 777 0 0 /tmp/PDiagAtt.vc
		DSMOptions ---- 777 0 0 /tmp/DSMOptions
		DSMOptions.vc ---- 777 0 0 /tmp/DSMOptions.vc
		FRUB	---- 777 0 0 /tmp/FRUB
		FRUs	---- 777 0 0 /tmp/FRUs
		TMInput	---- 777 0 0 /tmp/TMInput
		CDiagDev ---- 777 0 0 /tmp/CDiagDev
		MenuGoal ---- 777 0 0 /tmp/MenuGoal
		DAVars	---- 777 0 0 /tmp/DAVars
		DSMenu	---- 777 0 0 /tmp/DSMenu
		$
	stanzas d--- 777 0 0
		cdfiles.add ---- 777 0 0 /tmp/cdfiles.add
		$
	vg	d--- 777 0 0
		$
	$
lib	l--- 755 0 0 /usr/lib
SPOT	d--- 755 0 0
	$
sbin	d--- 755 0 0
	rc.boot   ---- 777 0 0 /tmp/rc.boot
	$
tmp	d--- 777 0 0
	$
usr	d--- 755 0 0
	tmp	d--- 755 0 0
	$
	lpp	d--- 755 0 0
		diagnostics	d--- 755 0 0
				da	d--- 755 0 0
				$
				data	d--- 755 0 0
				$
				bin	d--- 755 0 0
				$
				slih	d--- 755 0 0
				$
				catalog	d--- 755 0 0
					default d--- 755 0 0
					$
				$
		$
	$
	bin	d--- 755 0 0
		sh      ---- 555 0 0 /usr/bin/bsh
		odmadd  ---- 555 0 0 /usr/bin/odmadd
		ln	---- 555 0 0 /usr/bin/ln
		rm	---- 555 0 0 /usr/bin/rm
		tty	---- 555 0 0 /usr/bin/tty
		$
	lib	d--- 755 0 0
		objrepos l--- 777 0 0 /etc/objrepos
		drivers	d--- 777 0 0
        		cfs.ext      ---- 555 0 0 /usr/lib/drivers/cfs.ext
			hscsidd   ---- 777 0 0 /usr/lib/drivers/hscsidd
			pscsidd   ---- 777 0 0 /usr/lib/drivers/pscsidd
			pscsiddpin   ---- 777 0 0 /usr/lib/drivers/pscsiddpin
			scdisk      ---- 777 0 0 /usr/lib/drivers/scdisk
			scdiskpin   ---- 777 0 0 /usr/lib/drivers/scdiskpin
			ascsidd	   ---- 777 0 0 /usr/lib/drivers/ascsidd
			ascsiddpin ---- 777 0 0 /usr/lib/drivers/ascsiddpin
			fd	   ---- 777 0 0 /usr/lib/drivers/fd
			vscsidd    ---- 777 0 0 /usr/lib/drivers/vscsidd
			vscsiddpin ---- 777 0 0 /usr/lib/drivers/vscsiddpin
			pscsi720dd ---- 777 0 0 /usr/lib/drivers/pscsi720dd
			pscsi720ddpin ---- 777 0 0 /usr/lib/drivers/pscsi720ddpin
			$
		libc.a	 ---- 555 0 0 /usr/ccs/lib/libc.a.min
		libcfg.a ---- 555 0 0 /tmp/libcfg.a
		libodm.a ---- 555 0 0 /tmp/libodm.a
		libs.a   ---- 555 0 0 /usr/lib/libs.a.min
		methods d--- 777 0 0
			sdd	 ---- 777 0 0 /usr/lib/methods/sdd
			cfgfdd	 ---- 777 0 0 /usr/lib/methods/cfgfdd
			cfgfda	 ---- 777 0 0 /usr/lib/methods/cfgfda
			cfgbus   ---- 777 0 0 /usr/lib/methods/cfgbus
			cfghscsi  ---- 777 0 0 /usr/lib/methods/cfghscsi
			cfgsccd ---- 777 0 0 /usr/lib/methods/cfgsccd
			cfgsio   ---- 777 0 0 /usr/lib/methods/cfgsio
			cfgsys   ---- 777 0 0 /usr/lib/methods/cfgsys
			cfgsys_p   ---- 777 0 0 /usr/lib/methods/cfgsys_p
			chggen  ---- 777 0 0 /usr/lib/methods/chggen
			define   ---- 777 0 0 /usr/lib/methods/define
			defsys   ---- 777 0 0 /usr/lib/methods/defsys
			showled	---- 777 0 0 /usr/lib/methods/showled
			ucfgdevice  ---- 777 0 0 /usr/lib/methods/ucfgdevice
			ucfgcommo  ---- 777 0 0 /usr/lib/methods/ucfgcommo
			cfgascsi ---- 777 0 0 /usr/lib/methods/cfgascsi
			cfgvscsi ---- 777 0 0 /usr/lib/methods/cfgvscsi
			cfgpscsi ---- 777 0 0 /usr/lib/methods/cfgpscsi
			instdbcln ---- 777 0 0 /usr/lib/methods/instdbcln
			$
		microcode d--- 777 0 0
			8d77.32.54  ---- 777 0 0 /usr/lib/microcode/8d77.32.54
			8d77.42.58  l--- 777 0 0 /usr/lib/microcode/8d77.44.58
			8d77.44.58  ---- 777 0 0 /usr/lib/microcode/8d77.44.58
			$
		$
	share	d--- 755 0 0
		lib	d--- 755 0 0
		$
	$
	sbin	d--- 755 0 0
		bootinfo   ---- 777 0 0 /usr/sbin/bootinfo
        	mount      ---- 555 0 0 /usr/sbin/mount
		$
	$
$
