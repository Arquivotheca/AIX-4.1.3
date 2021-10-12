# @(#)01	1.3  src/bos/diag/boot/graphics3.dep.S, diagboot, bos41J, 9515A_all 4/5/95 15:22:13
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
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
########################################################################
./usr/lib/methods/cfg_fga
./usr/lib/methods/cfg_gga
./usr/lib/methods/cfg_hga
./usr/lib/methods/cfg_iga
./usr/lib/methods/cfg_wfg
./usr/lib/drivers/pci/fgadd
./usr/lib/drivers/pci/fgadd_pin
./usr/lib/drivers/pci/ggadd
./usr/lib/drivers/pci/ggadd_pin
./usr/lib/drivers/pci/ggadd_video
./usr/lib/drivers/pci/hgadd
./usr/lib/drivers/pci/hgadd_pin
./usr/lib/drivers/pci/igadd
./usr/lib/drivers/pci/igadd_pin
./usr/lib/drivers/pci/wfgdd
./usr/lib/drivers/pci/wfgdd_pin
