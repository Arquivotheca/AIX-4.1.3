# @(#)15	1.8  src/bos/diag/boot/tapediag6B.dep.S, diagboot, bos41J, 9523C_all 6/7/95 13:06:01
# COMPONENT_NAME: DIAGBOOT
#
# FUNCTIONS: Diagnostic tape 6B dependency file
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
#	mpa
./usr/lib/methods/cfgmpaa
./usr/lib/methods/cfgdmpa
./usr/lib/drivers/dmpadd
./usr/lpp/diagnostics/da/dampa
./usr/lpp/diagnostics/catalog/default/dampa.cat
