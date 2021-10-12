# @(#)16        1.1  src/bos/kernext/dartic/Makefile, dd_artic, bos411, 9428A410j 11/11/93 15:19:12
#  
#  COMPONENT_NAME: dd_artic -- ARTIC Diagnostic Device driver.
#
#  FUNCTIONS: Makefile
#
#  ORIGINS: 27
#
#  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#  combined with the aggregated modules for this product)
#                   SOURCE MATERIALS
#  (C) COPYRIGHT International Business Machines Corp. 1992, 1993
#  All Rights Reserved
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

KERNEL_EXT      = darticdd
ILIST           =  darticdd
IDIR		= /usr/lib/drivers/

LIBS            = ${KERNEXT_LIBS}

darticdd_ENTRYPOINT = darticconfig

darticdd_OFILES = \
	        darticdd.o


.include <${RULES_MK}>

