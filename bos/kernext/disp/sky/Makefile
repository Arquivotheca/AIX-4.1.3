# @(#)02	1.5  src/bos/kernext/disp/sky/Makefile, sysxdispsky, bos411, 9428A410j 10/28/93 08:35:08
#
#   COMPONENT_NAME: sysxdispsky
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1988,1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

EXPINC_SUBDIRS  = inc

KERNEL_EXT	= skydd
IDIR		= /usr/lib/drivers/
ILIST		= skydd

LIBS		= ${KERNEXT_LIBS} 
INCFLAGS        =-I./inc

vtt2drvr.o_CFLAGS       = -D_KERNEL -qmaxmem=7000

skydd_ENTRYPOINT	= sky_config

skydd_OFILES	= vtt2intr.o vtt2drvr.o skyerr.o

.include <${RULES_MK}>
