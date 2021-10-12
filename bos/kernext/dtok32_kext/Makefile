# @(#)48	1.3  src/bos/kernext/dtok32_kext/Makefile, tu_mps, bos411, 9438C411a 9/22/94 22:37:29
#
#   COMPONENT_NAME: tu_mps
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

KERNEL_EXT   = dtok32_kext

dtok32_kext_EXPORTS    = -bE:dtok32_kext.exp

dtok32_kext_OFILES     =  dtok32_kext.o 

INCFLAGS   = -I${MAKETOP}/bos/diag/tu/mps

LIBS       = ${KERNEXT_LIBS}

#CFLAGS	   =	-D_KERNEL -D_POWER_MP -DDEBUG

ILIST      = ${KERNEL_EXT}
IDIR       = /usr/lib/drivers/

.include <${RULES_MK}>

