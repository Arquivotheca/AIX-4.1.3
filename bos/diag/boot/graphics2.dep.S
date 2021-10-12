# @(#)94	1.6  src/bos/diag/boot/graphics2.dep.S, diagboot, bos41J, 9510A_all 3/3/95 13:51:07
#
#
# COMPONENT_NAME: DIAGBOOT
#
# FUNCTIONS: Config stuff
#	     This file has all the methods and drivers needed
#	     to configure graphics adapters. This file will be passed
#	     to restbyname to only let a certain number of files to
#	     be restored into the RAM file system.
#		
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
#	Gt0
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
#	GXT 1000
./usr/lib/methods/cfg_graphics
./usr/lib/methods/cfgrby
./usr/lib/drivers/rbydd
./usr/lib/microcode/8fbccp.00.00
./usr/lib/microcode/8fbcap.00.00
./usr/lib/microcode/8fbccp1.00.00
./usr/lib/microcode/8fbcap1.00.00
./usr/lib/microcode/8fbclogp.00.00
./usr/lib/microcode/8fbclogp1.00.00
./usr/lib/microcode/8fbchigp.00.00
./usr/lib/microcode/8fbchigp1.00.00
./usr/lpp/diagnostics/da/druby
