# @(#)82	1.9  src/bos/diag/boot/config2.dep.S, diagboot, bos41J, 9521B_all 5/25/95 10:07:26
#
# COMPONENT_NAME: DIAGBOOT
#
# FUNCTIONS: Configuration dependency file.
#	     This file has all the methods and drivers needed
#	     to configure native serial port and console.
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
#       system console
./usr/lib/methods/cfgcon
#       async
./usr/lib/methods/rsddload
./usr/lib/methods/cxload
./usr/lib/methods/cfgasync
./usr/lib/methods/ucfgasync
./usr/lib/methods/cfgtty
./usr/lib/methods/chgtty
./usr/lib/methods/ucfgtty
./usr/lib/methods/starttty
./usr/lib/drivers/liondd
./usr/lpp/diagnostics/da/dasync
#	kts
./usr/lpp/diagnostics/da/dkeytaba
./usr/lpp/diagnostics/da/dkmta
./usr/lpp/diagnostics/da/dkbd
./usr/lpp/diagnostics/da/dkbdsal
./usr/lpp/diagnostics/slih/kbd_slih
#   	128 sync stuff
./usr/lib/methods/cfgsync
./usr/lib/methods/ucfgsync
./usr/lib/methods/cfgcxma
./usr/lib/methods/ucfgcxma
./usr/lib/methods/chgcxma
./usr/lib/drivers/cxmadd
./usr/lbin/tty/stty-cxma
./usr/lib/microcode/cxbios.02.09
./usr/lib/microcode/cxfep.02.09
