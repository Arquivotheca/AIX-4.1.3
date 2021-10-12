# @(#)14	1.8  src/bos/diag/boot/tapediag6A.dep.S, diagboot, bos41J, 9523A_all 5/31/95 10:11:16
# COMPONENT_NAME: DIAGBOOT
#
# FUNCTIONS: Diagnostic tape 6A dependency file
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
#	enet
./usr/lib/methods/cfgddent
./usr/lib/drivers/entdiag
./usr/lpp/diagnostics/da/denet
./usr/lpp/diagnostics/catalog/default/denet.cat
./usr/lpp/diagnostics/bin/ulan
./usr/lpp/diagnostics/catalog/default/ulan.cat
./usr/lpp/diagnostics/bin/uenet
./usr/lpp/diagnostics/bin/stiluenet
./usr/lpp/diagnostics/bin/saluenet
./usr/lpp/diagnostics/catalog/default/uenet.cat
#	integrated Ethernet
./usr/lib/methods/cfgddeth
./usr/lib/drivers/ethdiag
./usr/lpp/diagnostics/da/dsaleth
./usr/lpp/diagnostics/da/dstileth
./usr/lpp/diagnostics/catalog/default/dstileth.cat
./usr/lpp/diagnostics/catalog/default/dsaleth.cat
#	mpqp
./usr/lib/methods/cfgmpqp
./usr/lib/methods/cfgmpqd
./usr/lib/methods/defmpqd
./usr/lib/drivers/mpqpdd
./usr/lib/asw/mpqpasw
#	token ring
./usr/lib/methods/cfgddtok
./usr/lib/drivers/tokdiag
./usr/lib/microcode/8fc8m.00.02
./usr/lib/microcode/8fc8p.00.01
./usr/lpp/diagnostics/da/dtoken
./usr/lpp/diagnostics/catalog/default/dtoken.cat
#	mps
./usr/lib/methods/cfgmps
./usr/lib/drivers/mpsdd
./usr/lib/drivers/dtok32_kext
./usr/lpp/diagnostics/da/dtok32
./usr/lpp/diagnostics/catalog/default/dtok32.cat
./usr/lpp/diagnostics/slih/tok32_intr
#	x25
./usr/lib/methods/cfgampx
./usr/lib/methods/cfgapm
./usr/lib/methods/chggen
./usr/lib/asw/geteibid.com
./usr/lib/microcode/f0efd.00.02
#	Artic
./usr/lib/methods/cfgdgric
./usr/lib/drivers/darticdd
./usr/lib/microcode/708fd.00.00
./usr/lpp/diagnostics/da/dartic
./usr/lpp/diagnostics/catalog/default/dartic.cat
./usr/lpp/diagnostics/catalog/default/dpm.cat
./usr/lpp/diagnostics/catalog/default/dx25.cat
./usr/lpp/diagnostics/catalog/default/dxm2p.cat
./usr/lpp/diagnostics/catalog/default/dmultp.cat
#	FDDI
./usr/lib/drivers/fddidiag
./usr/lib/drivers/fddidiagpin
./usr/lib/methods/cfgddfddi
./usr/lib/microcode/8ef4m.02.04
./usr/lib/microcode/fddi.diag
./usr/lpp/diagnostics/da/dfddi
./usr/lpp/diagnostics/catalog/default/dfddi.cat
