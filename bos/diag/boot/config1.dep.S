# @(#)81	1.4  src/bos/diag/boot/config1.dep.S, diagboot, bos41J, 9513A_all 3/21/95 12:55:44
#
#
# COMPONENT_NAME: DIAGBOOT
#
# FUNCTIONS: Config stuff
#	     This file has all the methods and drivers needed
#	     to configure tty, keyboard, mouse, tablet and lft.
#	     This file will be passed to restbyname to allow
#	     only certain files to be restored from the first
#	     backup image on tape.
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
#	tty
./usr/lib/drivers/rsdd
./usr/lib/drivers/tioc
./usr/lib/drivers/ldterm
./usr/lib/drivers/pse/pse
./usr/sbin/strload
#       lft
./usr/lib/methods/cfglft
./usr/lib/methods/startlft
./usr/lib/drivers/lft_loadpin
#	rcm
./usr/lib/methods/startrcm
./usr/lib/methods/cfgrcm
./usr/lib/drivers/rcm_loadpin
#	kts 
./usr/lib/drivers/tabletdd
./usr/lib/drivers/kbddd
./usr/lib/drivers/ktsdd
./usr/lib/methods/cfgkbd_std
#	mouse
./usr/lib/methods/cfgktsm
./usr/lib/methods/cfgkma
./usr/lib/drivers/mousedd
./usr/lib/methods/cfgmouse_std
./usr/lib/methods/cfgtablet_std
