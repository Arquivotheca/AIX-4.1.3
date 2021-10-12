# @(#)03	1.6  src/bos/diag/boot/rspc.cdrom.proto.S, diagboot, bos41J, 9523A_all 5/31/95 15:18:58
#
#   COMPONENT_NAME: DIAGBOOT
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
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
	graphics3.dep ---- 555 0 0 /tmp/graphics3.dep
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
		rspc.cdfiles.add ---- 777 0 0 /tmp/rspc.cdfiles.add
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
		sleep   ---- 555 0 0 /usr/bin/sleep
		tty	---- 555 0 0 /usr/bin/tty
		$
	lib	d--- 755 0 0
		objrepos l--- 777 0 0 /etc/objrepos
		drivers	d--- 777 0 0
        		cfs.ext      ---- 555 0 0 /usr/lib/drivers/cfs.ext
			isa	d--- 755 0 0
				rspcios ---- 777 0 0 /usr/lib/drivers/isa/rspcios
				fdisa	---- 777 0 0 /usr/lib/drivers/isa/fdisa
				$
			pci	d--- 755 0 0
				ncr810dd ---- 777 0 0 /usr/lib/drivers/pci/ncr810dd
				$
			scdisk      ---- 777 0 0 /usr/lib/drivers/scdisk
			scdiskpin   ---- 777 0 0 /usr/lib/drivers/scdiskpin
			fd	---- 777 0 0 /usr/lib/drivers/fd
			$
		libc.a	 ---- 555 0 0 /usr/ccs/lib/libc.a.min
		libcfg.a ---- 555 0 0 /tmp/libcfg.a
		libodm.a ---- 555 0 0 /tmp/libodm.a
		libs.a   ---- 555 0 0 /usr/lib/libs.a.min
		methods d--- 777 0 0
			cfgbus_pci ---- 777 0 0 /usr/lib/methods/cfgbus_pci
			cfgbus_isa ---- 777 0 0 /usr/lib/methods/cfgbus_isa
			cfgsio_isa   ---- 777 0 0 /usr/lib/methods/cfgsio_isa
			cfgncr_scsi ---- 777 0 0 /usr/lib/methods/cfgncr_scsi
			cfgsccd ---- 777 0 0 /usr/lib/methods/cfgsccd
			cfgsys_rspc   ---- 777 0 0 /usr/lib/methods/cfgsys_rspc
			cfgfda_isa ---- 777 0 0 /usr/lib/methods/cfgfda_isa
			cfgfdd ---- 777 0 0 /usr/lib/methods/cfgfdd
			chggen  ---- 777 0 0 /usr/lib/methods/chggen
			chggen_rspc  ---- 777 0 0 /usr/lib/methods/chggen_rspc
			define   ---- 777 0 0 /usr/lib/methods/define
			define_rspc   ---- 777 0 0 /usr/lib/methods/define_rspc
			defsys   ---- 777 0 0 /usr/lib/methods/defsys
			showled	---- 777 0 0 /usr/lib/methods/showled
			ucfgdevice  ---- 777 0 0 /usr/lib/methods/ucfgdevice
			instdbcln ---- 777 0 0 /usr/lib/methods/instdbcln
			$
		microcode d--- 777 0 0
			$
		$
	share	d--- 755 0 0
		lib	d--- 755 0 0
		$
	$
	sbin	d--- 755 0 0
		bootinfo   ---- 777 0 0 /usr/sbin/bootinfo
        	mount      ---- 555 0 0 /usr/sbin/mount
		reboot	---- 555 0 0 /usr/sbin/reboot
		$
	$
$
