# @(#)11	1.2  src/bos/diag/boot/tapediag3B.dep.S, diagboot, bos411, 9440C411a 10/7/94 16:45:19
#
# COMPONENT_NAME: DIAGBOOT
#
# FUNCTIONS: Diagnostic tape 3B dependency file
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
./usr/lib/methods/cfg_graphics
./usr/lib/methods/cfg_mid
./usr/lib/drivers/middd_load
./usr/lib/drivers/middd_loadpin
#	ped
./usr/lib/microcode/8ee3xb.01.01
./usr/lib/microcode/8ee3nb.01.01
./usr/lib/microcode/8ee3p4d1.00.01
./usr/lib/microcode/8ee3p4d2.00.01
./usr/lib/microcode/8ee3p4d3.00.01
./usr/lib/microcode/8ee3p4d4.00.01
./usr/lib/microcode/8ee3p4d5.00.01
#	lega
./usr/lib/microcode/8ee3lb.00.01
./usr/lib/microcode/8ee3l2b.00.01
./usr/lib/microcode/8ee3ld1.00.04
./usr/lib/microcode/8ee3ld2.00.04
./usr/lib/microcode/8ee3ld3.00.04
./usr/lib/microcode/8ee3ld4.00.04
./usr/lib/microcode/8ee3ld5.00.04
./usr/lpp/diagnostics/da/dped
./usr/lpp/diagnostics/da/dlega
./usr/lpp/diagnostics/catalog/default/ped.cat
./usr/lpp/diagnostics/catalog/default/lega2.cat
