# @(#)09        1.38  src/bos/diag/da/Makefile, cmddiag, bos41J, 9520B_all 5/19/95 09:20:17
#
#   COMPONENT_NAME: CMDDIAG
#
#   FUNCTIONS:
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

SUBDIRS         = 370pca bblue async cdrom corvette \
	          dca dial disks eth ethsal ethstw fd \
		  fddi fpa gem gio kbd iop iopsl \
		  lpfk mag msla mcode memory morps mouse mpa \
	          niopp scsi scsitapes sdisk siosl \
	          sol sys tablet tok tok32 sgasal wga \
	          niokta lega ped sky swmono artic ruby nep \
		  baud d7135 pcitok gga pciscsi pcient resid

# WARNING - niokta MUST be built AFTER siosl, DO NOT REORDER

.include <${RULES_MK}>
