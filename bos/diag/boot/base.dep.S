# @(#)75	1.8  src/bos/diag/boot/base.dep.S, diagboot, bos41J, 9517B_all 4/24/95 15:13:58
#
#
# COMPONENT_NAME: DIAGBOOT
#
# FUNCTIONS: Config stuff
#	     This file has all the base programs needed to run
# 	     the base system. This include the diagnostic controller
#	     Executable that needs to stay in RAM file system at
#	     all time, also can go in here.
#		
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
########################################################################
./usr/lib/drivers/diagex
./usr/lib/drivers/rcm_load
./usr/lpp/diagnostics/bin/dctrl
# The next two drivers are top portion of the drivers
# and need to be in the RAM file system at all times
# since the adapters stay configured.
./usr/lib/drivers/scarray
#./usr/lib/drivers/ssa
./usr/sbin/mkdev
./usr/sbin/rmdev
./usr/bin/compress
./usr/lpp/diagnostics/bin/diags
./usr/lpp/diagnostics/bin/dutil
./usr/lpp/diagnostics/bin/diagpt
./usr/lpp/diagnostics/da/dmemory
./usr/lpp/diagnostics/da/dsys
./usr/lpp/diagnostics/da/diop
./usr/lpp/diagnostics/da/diopsl
./usr/lpp/diagnostics/da/dfpp
./usr/lpp/diagnostics/catalog/default/dcda.cat
./usr/lpp/diagnostics/catalog/default/dctrl.cat
./usr/lpp/diagnostics/catalog/default/diags.cat
./usr/lpp/diagnostics/catalog/default/dutil.cat
./usr/lpp/diagnostics/catalog/default/memory.cat
./usr/lpp/diagnostics/catalog/default/sys.cat
./usr/lpp/diagnostics/catalog/default/fpp.cat
./usr/lpp/diagnostics/catalog/default/diop.cat
./usr/lpp/diagnostics/catalog/default/udiskmnt.cat
./usr/lpp/diagnostics/catalog/default/ubackrest.cat
./usr/lpp/diagnostics/catalog/default/ugenucodesa.cat
./usr/lpp/diagnostics/catalog/default/u7135.cat
./usr/lib/methods/graphdev.cat
#./usr/lib/nls/msg/en_US/sa.cat
#./usr/lib/nls/msg/en_US/ssadisk.cat
#./usr/lib/nls/msg/en_US/SSAaids.cat
