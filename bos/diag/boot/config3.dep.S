# @(#)83	1.4  src/bos/diag/boot/config3.dep.S, diagboot, bos41J, 9515A_all 4/5/95 10:11:53
#
# COMPONENT_NAME: DIAGBOOT
#
# FUNCTIONS: Configuration dependency file.
#	     This file has all the methods and drivers needed
#	     to configure all Media devices on the system.
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
#	kazusa
./usr/lib/methods/cfgbadisk
./usr/lib/drivers/badisk
./usr/lib/drivers/badiskpin
#	SCSI adapter
./usr/lib/microcode/8d77.a0.12
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
#	cdrom
./usr/lib/methods/cfgsccd
#	disks
./usr/lib/methods/cfgscdisk
./usr/lib/drivers/scdisk
./usr/lib/drivers/scdiskpin
#	serial disks
./usr/lib/methods/cfgserdasda
./usr/lib/methods/cfgserdasdc
./usr/lib/methods/cfgserdasdd
./usr/lib/methods/sdd
./usr/lib/drivers/sd
./usr/lib/drivers/sdpin
./usr/lib/microcode/8f78.00.16
#	disk array
./usr/lib/drivers/scarraypin
./usr/lib/methods/cfgscarray
./usr/lib/methods/cfgdac
./usr/lib/methods/cfgdar
./usr/lib/methods/chgdar
./usr/lib/methods/defscarray
#./usr/lib/methods/ucfgdac
./usr/lib/methods/ucfgdar
./usr/lib/methods/darcfgrule
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
