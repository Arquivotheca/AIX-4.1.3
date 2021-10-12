# @(#)02	1.14  src/bos/diag/boot/tape.proto.S, diagboot, bos41J, 9517B_all 4/24/95 15:14:12
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
/bootrec 14000 0
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
	base.dep	---- 777 0 0 /tmp/base.dep
	config1.dep	---- 777 0 0 /tmp/config1.dep
	config2.dep	---- 777 0 0 /tmp/config2.dep
	config3.dep	---- 777 0 0 /tmp/config3.dep
	config4.dep	---- 777 0 0 /tmp/config4.dep
	config5.dep	---- 777 0 0 /tmp/config5.dep
	graphics1.dep	---- 777 0 0 /tmp/graphics1.dep
	graphics2.dep	---- 777 0 0 /tmp/graphics2.dep
	removed_graphics.dep ---- 777 0 0 /tmp/removed_graphics.dep
	tapediag3A.dep	---- 777 0 0 /tmp/tapediag3A.dep
	tapediag3B.dep	---- 777 0 0 /tmp/tapediag3B.dep
	tapediag3C.dep	---- 777 0 0 /tmp/tapediag3C.dep
	tapediag5.dep	---- 777 0 0 /tmp/tapediag5.dep
	tapediag6A.dep	---- 777 0 0 /tmp/tapediag6A.dep
	tapediag6B.dep	---- 777 0 0 /tmp/tapediag6B.dep
	tapediag7A.dep	---- 777 0 0 /tmp/tapediag7A.dep
	tapediag7B.dep	---- 777 0 0 /tmp/tapediag7B.dep
	tapediag8.dep	---- 777 0 0 /tmp/tapediag8.dep
	linkedfiles.dep ---- 777 0 0 /tmp/linkedfiles.dep
	restore_files	---- 777 0 0 /tmp/restore_files
	rmfiles		---- 777 0 0 /tmp/rmfiles
	profile		---- 555 0 0 /etc/profile
	passwd		---- 555 0 0 /etc/passwd
	drivers	l--- 777 0 0 /usr/lib/drivers
	consdef	---- 777 0 0 /etc/consdef
	init    ---- 777 0 0 /usr/lib/boot/ssh
	cfgmgr	---- 777 0 0 /usr/lpp/diagnostics/bin/cfgmgr
	chkdskt	---- 777 0 0 /usr/lpp/diagnostics/bin/chkdskt
	methods l--- 777 0 0 /usr/lib/methods
	microcode l--- 777 0 0 /usr/lib/microcode
	lpp	d--- 755 0 0
		diagnostics l--- 755 0 0 /usr/lpp/diagnostics
		$
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
		tapefiles.add ---- 777 0 0 /tmp/tapefiles.add
		$
	$
lib	l--- 755 0 0 /usr/lib
sbin	d--- 755 0 0
	rc.boot   ---- 777 0 0 /tmp/rc.boot
	$
tmp	d--- 777 0 0
	$
usr	d--- 755 0 0
	tmp	d--- 755 0 0
	$
	bin	d--- 755 0 0
		rm	---- 777 0 0 /usr/bin/rm
		cut	---- 777 0 0 /usr/bin/cut
		dspmsg  ---- 777 0 0 /usr/bin/dspmsg
		ln	---- 777 0 0 /usr/bin/ln
		odmadd    ---- 777 0 0 /usr/bin/odmadd
		odmdelete ---- 777 0 0 /usr/bin/odmdelete
		odmget    ---- 777 0 0 /usr/bin/odmget
		cpio	---- 777 0 0 /usr/bin/cpio
		mv	---- 777 0 0 /usr/bin/mv
		grep	---- 777 0 0 /usr/bin/grep
		bsh	l--- 555 0 0 /usr/bin/sh
		sh      ---- 555 0 0 /usr/bin/bsh
		tctl    ---- 777 0 0 /usr/bin/tctl
		sleep	---- 777 0 0 /usr/bin/sleep
		tty	---- 777 0 0 /usr/bin/tty
		$
	lib	d--- 755 0 0
		objrepos l--- 777 0 0 /etc/objrepos
		drivers	d--- 777 0 0
			fd	 ---- 777 0 0 /usr/lib/drivers/fd
			hscsidd   ---- 777 0 0 /usr/lib/drivers/hscsidd
			pscsidd   ---- 777 0 0 /usr/lib/drivers/pscsidd
			pscsiddpin ---- 777 0 0 /usr/lib/drivers/pscsiddpin
			tape      ---- 777 0 0 /usr/lib/drivers/tape
			tapepin   ---- 777 0 0 /usr/lib/drivers/tapepin
			ascsidd  ---- 777 0 0 /usr/lib/drivers/ascsidd
			ascsiddpin  ---- 777 0 0 /usr/lib/drivers/ascsiddpin
			vscsidd  ---- 777 0 0 /usr/lib/drivers/vscsidd
			vscsiddpin  ---- 777 0 0 /usr/lib/drivers/vscsiddpin
			pscsi720dd   ---- 777 0 0 /usr/lib/drivers/pscsi720dd
			pscsi720ddpin ---- 777 0 0 /usr/lib/drivers/pscsi720ddpin
			$
		libc.a	 ---- 555 0 0 /usr/ccs/lib/libc.a.min
		libcfg.a ---- 555 0 0 /tmp/libcfg.a
		libodm.a ---- 555 0 0 /tmp/libodm.a
		libs.a   ---- 555 0 0 /usr/lib/libs.a.min
		libdiag.a ---- 555 0 0 /tmp/libdiag.a
		libasl.a  ---- 555 0 0 /tmp/libasl.a
		libcur.a  ---- 555 0 0 /tmp/libcur.a
		boot	d--- 777 0 0
			chramfs ---- 777 0 0 /usr/lib/boot/chramfs
			$
		methods d--- 777 0 0
			cfgfdd	---- 777 0 0 /usr/lib/methods/cfgfdd
			cfgfda	---- 777 0 0 /usr/lib/methods/cfgfda
			cfgdiag  ---- 777 0 0 /usr/lib/methods/cfgdiag
			ucfgdiag ---- 777 0 0 /usr/lib/methods/ucfgdiag
			cfgbus   ---- 777 0 0 /usr/lib/methods/cfgbus
			cfghscsi  ---- 777 0 0 /usr/lib/methods/cfghscsi
			cfgpscsi  ---- 777 0 0 /usr/lib/methods/cfgpscsi
			cfgascsi  ---- 777 0 0 /usr/lib/methods/cfgascsi
			cfgvscsi  ---- 777 0 0 /usr/lib/methods/cfgvscsi
			cfgsctape ---- 777 0 0 /usr/lib/methods/cfgsctape
			cfgsio   ---- 777 0 0 /usr/lib/methods/cfgsio
			cfgsys   ---- 777 0 0 /usr/lib/methods/cfgsys
			cfgsys_p   ---- 777 0 0 /usr/lib/methods/cfgsys_p
			chggen  ---- 777 0 0 /usr/lib/methods/chggen
			define   ---- 777 0 0 /usr/lib/methods/define
			defsys   ---- 777 0 0 /usr/lib/methods/defsys
			instdbcln  ---- 777 0 0 /usr/lib/methods/instdbcln
			showled	---- 777 0 0 /usr/lib/methods/showled
			undefine ---- 777 0 0 /usr/lib/methods/undefine
			ucfgdevice  ---- 777 0 0 /usr/lib/methods/ucfgdevice
			ucfgcommo  ---- 777 0 0 /usr/lib/methods/ucfgcommo
			devices.cat ---- 777 0 0 /usr/lib/methods/devices.cat
			fddi.cat  ---- 777 0 0 /usr/lib/methods/fddi.cat
			scdisk.cat ---- 777 0 0 /usr/lib/methods/scdisk.cat
			devices_tok.cat ---- 777 0 0 /usr/lib/methods/devices_tok.cat
			scarray.cat ---- 777 0 0 /usr/lib/methods/scarray.cat
			$
		microcode d--- 777 0 0
			8d77.32.54  ---- 777 0 0 /usr/lib/microcode/8d77.32.54
			8d77.42.58  l--- 777 0 0 /usr/lib/microcode/8d77.44.58
			8d77.44.58  ---- 777 0 0 /usr/lib/microcode/8d77.44.58
			$
		nls	d--- 755 0 0
			loc	d--- 755 0 0
				C.lftkeymap	---- 777 0 0 /usr/lib/nls/loc/C.lftkeymap
				$
			$
		$
	lpp	d--- 755 0 0
		diagnostics   d--- 755 0 0
			bin	d--- 755 0 0
				$
			da	d--- 755 0 0
				$
			catalog	d--- 755 0 0
				default	d--- 755 0 0
					$
				$
			slih	d--- 755 0 0
				$
			data	d--- 755 0 0
				async.add     ---- 777 0 0 /tmp/async.add
				display.add   ---- 777 0 0 /tmp/display.add
				inputdev.add  ---- 777 0 0 /tmp/inputdev.add
				commo.add     ---- 777 0 0 /tmp/commo.add
				media.add     ---- 777 0 0 /tmp/media.add
				diagodm.add   ---- 777 0 0 /tmp/diagodm.add
				term.cpio     ---- 777 0 0 /tmp/term.cpio
				disks.add     ---- 777 0 0 /tmp/disks.add
				cdrom.add     ---- 777 0 0 /tmp/cdrom.add
				$
			$
		fonts	d--- 755 0 0
			Erg22.iso1.snf	---- 755 0 0 /usr/lpp/fonts/Erg22.iso1.snf
			Erg11.iso1.snf	---- 755 0 0 /usr/lpp/fonts/Erg11.iso1.snf
			$
		$
	share	d--- 755 0 0
		lib	d--- 755 0 0
			terminfo  d--- 755 0 0
			$
		$
	$
	sbin	d--- 755 0 0
		bootinfo   ---- 777 0 0 /usr/sbin/bootinfo
		restbyname ---- 555 0 0 /usr/sbin/restbyname
		slibclean ---- 555 0 0 /usr/sbin/slibclean
		lsdev	  ---- 555 0 0 /usr/sbin/lsdev
		flattendb ---- 555 0 0 /tmp/flattendb
		$
	$
$
