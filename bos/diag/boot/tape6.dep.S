# @(#)06	1.12  src/bos/diag/boot/tape6.dep.S, diagboot, bos41J, 9523C_all 6/7/95 13:05:59
# COMPONENT_NAME: DIAGBOOT
#
# FUNCTIONS: Diagnostic tape 6A dependency file
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#	enet
./usr/lib/methods/cfgddent
./usr/lib/methods/ucfgent
./usr/lib/drivers/entdiag
./usr/lpp/diagnostics/da/denet
./usr/lpp/diagnostics/catalog/default/denet.cat
./usr/lpp/diagnostics/bin/ulan
./usr/lpp/diagnostics/catalog/default/ulan.cat
./usr/lpp/diagnostics/bin/uenet
./usr/lpp/diagnostics/bin/saluenet
./usr/lpp/diagnostics/bin/stiluenet
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
./usr/lib/methods/ucfgtok
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
#	mpa
./usr/lib/methods/cfgmpaa
./usr/lib/methods/cfgdmpa
./usr/lib/drivers/dmpadd
./usr/lpp/diagnostics/da/dampa
./usr/lpp/diagnostics/catalog/default/dampa.cat
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
#	3270
./usr/lib/methods/cfg3270
./usr/lib/drivers/c327dd
./usr/lpp/diagnostics/da/ddca
./usr/lpp/diagnostics/catalog/default/ddca.cat
#	psla/msla
./usr/lib/methods/cfgpsla
./usr/lib/methods/cfgmsla
./usr/lib/methods/cfghia
./usr/lib/drivers/psladd
./usr/lib/drivers/hiadd
./usr/lpp/diagnostics/da/dsla
./usr/lpp/diagnostics/catalog/default/dsla.cat
./usr/lib/microcode/8787S.00.01
./usr/lib/microcode/8787d50.00.01
./usr/lib/microcode/8787dpl2.00.01
./usr/lib/microcode/8787dpl3.00.01
./usr/lib/microcode/8787dwrp.00.01
./usr/lib/microcode/8787dmod.00.01
./usr/lib/microcode/8787dp.00.01
./usr/lib/microcode/8787dd1.00.01
./usr/lib/microcode/8787dd2.00.01
#	Optics 2 port
./usr/lib/methods/cfgsol
./usr/lib/drivers/soldd
./usr/lib/drivers/solddpin
./usr/lpp/diagnostics/da/doptics
./usr/lpp/diagnostics/catalog/default/optics.cat
#	FDDI
./usr/lib/drivers/fddidiag
./usr/lib/drivers/fddidiagpin
./usr/lib/methods/cfgddfddi
./usr/lib/microcode/8ef4m.02.04
./usr/lib/microcode/fddi.diag
./usr/lpp/diagnostics/da/dfddi
./usr/lpp/diagnostics/catalog/default/dfddi.cat
#	370
./usr/lib/methods/cfgcat
./usr/lib/drivers/catdd
./usr/lib/microcode/fe92d.00.01
./usr/lib/microcode/fe92.00.03
./usr/lib/microcode/cu3088b.00.00
./usr/lib/microcode/cu3088e.00.00
./usr/lib/microcode/cu3172b.00.00
./usr/lpp/diagnostics/da/d370pc
./usr/lpp/diagnostics/catalog/default/d370pc.cat
