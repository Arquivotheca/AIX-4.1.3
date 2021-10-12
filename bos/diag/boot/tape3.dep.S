# @(#)04	1.10  src/bos/diag/boot/tape3.dep.S, diagboot, bos41J, 9513A_all 3/27/95 15:14:24
#
# COMPONENT_NAME: DIAGBOOT - TAPE DIAGNOSTIC PACKAGE
#
# FUNCTIONS: Diagnostic Tape 3A Content File
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
########################################################################
#	skyway
./usr/lib/methods/cfgsky
./usr/lib/drivers/skydd
./usr/lpp/diagnostics/da/dsky
./usr/lpp/diagnostics/da/dskym
./usr/lpp/diagnostics/catalog/default/dsky.cat
./usr/lpp/diagnostics/catalog/default/dskym.cat
#       sga
./usr/lpp/diagnostics/da/dsga
./usr/lpp/diagnostics/catalog/default/dsga.cat
./usr/lib/drivers/sgadd
./usr/lib/methods/cfgsga
./usr/lib/methods/cfg_sgabus
#       wga
./usr/lpp/diagnostics/da/dwga
./usr/lpp/diagnostics/catalog/default/dwga.cat
./usr/lib/drivers/wgadd_pin
./usr/lib/drivers/wgadd
./usr/lib/methods/cfg_wga
./usr/lib/methods/cfg_graphics
./usr/lib/methods/cfg_mid
./usr/lib/drivers/middd_load
./usr/lib/drivers/middd_loadpin
#	bblue
#./usr/lib/drivers/bbldd
#./usr/lib/drivers/bblddpin
#./usr/lib/methods/cfg_bbl
#./usr/lpp/diagnostics/da/dbblue
#./usr/lpp/diagnostics/catalog/default/dbblue.cat
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
#	gemini
./usr/lib/methods/cfggem
./usr/lib/drivers/gemdd
./usr/lib/microcode/8ffdgu.00.04
./usr/lib/microcode/8ffdgt.00.04
./usr/lib/microcode/8ffdgv.00.04
./usr/lib/microcode/8ffdvp.00.03
./usr/lib/microcode/8ffdc2.00.04
./usr/lib/microcode/8ffdsh.00.04
./usr/lib/microcode/8ffddc25.00.02
./usr/lib/microcode/8ffddbif.00.01
./usr/lib/microcode/8ffddgcp.00.02
./usr/lpp/diagnostics/da/dgraph
./usr/lpp/diagnostics/catalog/default/dgraph.cat
#	Ruby
./usr/lib/methods/cfgrby
./usr/lib/drivers/rbydd
./usr/lpp/diagnostics/da/druby
./usr/lpp/diagnostics/catalog/default/druby.cat
./usr/lib/microcode/8fbccp.00.00
./usr/lib/microcode/8fbcap.00.00
./usr/lib/microcode/8fbccp1.00.00
./usr/lib/microcode/8fbcap1.00.00
./usr/lib/microcode/8fbclogp.00.00
./usr/lib/microcode/8fbclogp1.00.00
./usr/lib/microcode/8fbchigp.00.00
./usr/lib/microcode/8fbchigp1.00.00
#	8f9a
#./usr/lib/drivers/nepdd_load
#./usr/lib/drivers/nepdd_loadpin
#./usr/lib/methods/cfg_nep
#./usr/lib/microcode/8f9a.00.00
#./usr/lpp/diagnostics/da/dnep
#./usr/lpp/diagnostics/catalog/default/dnep.cat
#	mag
#./usr/lib/methods/cfg_mag
#./usr/lib/drivers/magdd_load
#./usr/lib/drivers/magdd_loadpin
#./usr/lpp/diagnostics/da/dmag
#./usr/lpp/diagnostics/catalog/default/dmag.cat
