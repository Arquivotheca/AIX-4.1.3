# @(#)00	1.6  src/bos/diag/boot/removed_graphics.dep.S, diagboot, bos41J, 9513A_all 3/27/95 15:14:21
#
#
# COMPONENT_NAME: DIAGBOOT
#
# FUNCTIONS: Config stuff
#	     This file has all the methods and drivers that will
#	     be used by rc.boot to go through and remove
#	     after configuring the adapters to save RAM space.
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
#	color and grayscale display adapters
./usr/lib/methods/cfgsky
./usr/lib/drivers/skydd
#       sga
./usr/lib/drivers/sgadd
./usr/lib/methods/cfgsga
./usr/lib/methods/cfg_sgabus
#       wga
./usr/lib/drivers/wgadd_pin
./usr/lib/drivers/wgadd
./usr/lib/methods/cfg_wga
./usr/lib/methods/cfg_graphics
#	mag
#./usr/lib/drivers/magdd_loadpin
#./usr/lib/methods/cfg_mag
#	Gt4
./usr/lib/methods/cfg_mid
./usr/lib/drivers/middd_load
./usr/lib/drivers/middd_loadpin
#       4006 & 4007 & efc6
#./usr/lib/drivers/bblddpin
#./usr/lib/methods/cfg_bbl
#	Gt0
./usr/lib/methods/cfggem
./usr/lib/drivers/gemdd
#	GXT 1000
./usr/lib/methods/cfgrby
./usr/lib/drivers/rbydd
#	8f9a
#./usr/lib/methods/cfg_nep
#./usr/lib/drivers/nepdd_loadpin
