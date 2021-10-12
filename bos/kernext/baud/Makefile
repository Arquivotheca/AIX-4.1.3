
# @(#)54        1.1  , tu_products,  411
#
#  COMPONENT_NAME: foxclub diagnostics device driver makefile			
#
#  ORIGINS: 27
#
#  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#  combined with the aggregated modules for this product)
#                   SOURCE MATERIALS
#  (C) COPYRIGHT International Business Machines Corp. 1991, 1994
#  All Rights Reserved
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

#VPATH =  ${MAKETOP}../../kernext/baud

OPT_LEVEL = 

KERNEL_EXT = bauddiagdd bauddiagpin

bauddiagdd_ENTRYPOINT		= baud_config
bauddiagpin_ENTRYPOINT		= baud_intr_handler

bauddiagdd_OFILES =  \
	baud_open.o  		\
	baud_read.o		\
	baud_write.o		\
	baud_intr_handler.o	\
	baud_mpx.o		\
	baud_close.o 		\
	baud_ioctl.o		\
	baud_misc.o		\
	baud_watchdog.o		\
	baud_config.o

bauddiagpin_OFILES = \
	baud_intr_handler.o	\
	baud_misc.o 		\
	baud_watchdog.o

ILIST	= bauddiagdd bauddiagpin
IDIR    = /etc/drivers/


#IMPS = $(SYSX_IMPS)
#LIBS = $(SYSX_LIBS) \
#	/lib/kernex.exp \
#	/lib/syscalls.exp \
#	/lib/libodm.a \
#	/lib/libcsys.a \
#	/lib/libc.a \
#	/lib/libcfg.a

LIBS = ${KERNEXT_LIBS}

.include <${RULES_MK}>
