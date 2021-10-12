# @(#)05	1.10  src/bos/diag/boot/tape5.dep.S, diagboot, bos41J, 9517B_all 4/24/95 15:14:15
#
# COMPONENT_NAME: DIAGBOOT
#
# FUNCTIONS: Diagnostic tape 5 dependency file
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
./usr/lib/methods/chgdisk
#	kazusa
./usr/lib/methods/cfgbadisk
./usr/lib/drivers/badisk
./usr/lib/drivers/badiskpin
./usr/lpp/diagnostics/da/dhfd
./usr/lpp/diagnostics/catalog/default/dhfd.cat
./usr/lpp/diagnostics/bin/kazfor
./usr/lpp/diagnostics/catalog/default/kazfor.cat
#	alter bootlist 
./usr/lpp/diagnostics/bin/umblist
./usr/lpp/diagnostics/catalog/default/umblist.cat
./usr/bin/bootlist
#	SCSI adapter
./usr/lib/microcode/8d77.a0.16
./usr/lib/microcode/8d77.32.54
./usr/lib/microcode/8d77.44.54
./usr/lib/methods/cfghscsi
./usr/lib/methods/cfgpscsi
./usr/lib/methods/cfgascsi
./usr/lib/methods/cfgvscsi
./usr/lib/drivers/ascsidd
./usr/lib/drivers/ascsiddpin
./usr/lib/drivers/vscsidd
./usr/lib/drivers/vscsiddpin
./usr/lib/drivers/hscsidd
./usr/lib/drivers/pscsidd
./usr/lib/drivers/pscsiddpin
./usr/lib/drivers/pscsi720dd
./usr/lib/drivers/pscsi720ddpin
./usr/lpp/diagnostics/da/dscsia
./usr/lpp/diagnostics/catalog/default/dscsia.cat
./usr/lpp/diagnostics/bin/uscsi
./usr/lpp/diagnostics/catalog/default/uscsi.cat
./usr/lpp/diagnostics/da/dacorv
./usr/lpp/diagnostics/slih/corv_slih
./usr/lpp/diagnostics/catalog/default/dacorv.cat
#	cdrom
./usr/lib/methods/cfgsccd
./usr/lpp/diagnostics/da/dcdrom
./usr/lpp/diagnostics/catalog/default/dcdrom.cat
#	tape
./usr/lpp/diagnostics/da/dtape
./usr/lpp/diagnostics/catalog/default/datape.cat
#	disks
./usr/lib/methods/cfgscdisk
./usr/lib/drivers/scdisk
./usr/lib/drivers/scdiskpin
./usr/lpp/diagnostics/da/dhf
./usr/lpp/diagnostics/catalog/default/dhf.cat
./usr/lpp/diagnostics/catalog/default/dhflvm.cat
./usr/lpp/diagnostics/bin/uformat
./usr/lpp/diagnostics/catalog/default/ufmt.cat
./usr/lpp/diagnostics/bin/umcode
./usr/lpp/diagnostics/catalog/default/umcode.cat
./usr/bin/dosdir
#	serial disks
./usr/lib/methods/cfgserdasda
./usr/lib/methods/cfgserdasdc
./usr/lib/methods/cfgserdasdd
./usr/lib/methods/sdd
./usr/lib/drivers/sd
./usr/lib/drivers/sdpin
./usr/lib/microcode/8f78.00.16
./usr/lpp/diagnostics/da/dh2a
./usr/lpp/diagnostics/da/dh2cntrl
./usr/lpp/diagnostics/da/dh2dasd
./usr/lpp/diagnostics/bin/srwfor
./usr/bin/tar
./usr/lpp/diagnostics/catalog/default/dh2.cat
#	disk array
./usr/lib/drivers/scarraypin
./usr/lib/methods/cfgscarray
./usr/lib/methods/cfgdac
./usr/lib/methods/cfgdar
./usr/lib/methods/chgdar
./usr/lib/methods/defscarray
./usr/lib/methods/ucfgdac
./usr/lib/methods/ucfgdar
./usr/lib/methods/darcfgrule
./usr/lpp/diagnostics/da/d7135
./usr/lpp/diagnostics/catalog/default/d7135.cat
./usr/lpp/diagnostics/da/d7135.data
./usr/lpp/diagnostics/bin/u7135
./usr/lpp/diagnostics/bin/u7135.data
#	ssa
#./usr/lib/methods/cfgssa
#./usr/lib/methods/cfgssar
#./usr/lib/methods/cfgssadisk
#./usr/lib/methods/cfgssapdisk
#./usr/lib/methods/defssadev
#./usr/lib/methods/defssar
#./usr/lib/methods/udefssadev
#./usr/lib/drivers/ssadisk
#./usr/lib/drivers/ssadiskpin
#./usr/lib/drivers/ssapin
#./usr/lpp/diagnostics/da/dmayf
#./usr/lpp/diagnostics/bin/umayf
#./usr/lib/nls/msg/en_US/dmayf.cat
#./usr/lib/nls/msg/en_US/umayf.cat
#       diskette
./usr/lpp/diagnostics/da/dfda
./usr/lpp/diagnostics/da/dfd
./usr/lpp/diagnostics/catalog/default/dfd.cat
#	diskette utility 
./usr/lpp/diagnostics/bin/ufd
./usr/lpp/diagnostics/catalog/default/ufd.cat
#	backup and restore needs CEREADME file
./usr/sbin/backbyname
./usr/bin/ls
./usr/lpp/diagnostics/CEREADME
./usr/lpp/diagnostics/bin/ubackrest
#	generic microcode download
./usr/lpp/diagnostics/bin/ugenucodesa
#       disk copy
./usr/lpp/diagnostics/bin/udiskmnt
./usr/lpp/diagnostics/bin/udmedit
./usr/lpp/diagnostics/catalog/default/udmedit.cat
