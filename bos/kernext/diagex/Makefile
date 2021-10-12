# @(#)24	1.5  src/bos/kernext/diagex/Makefile, sysxdiag, bos411, 9431A411a 7/21/94 15:45:07
# COMPONENT_NAME: (sysxdiag) Diagnostics Kernel Extensions
#
# FUNCTIONS: N/A
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

OTHERS          = root/bos.diag.rte.trc.S

KERNEL_EXT	= diagex
IDIR            = /usr/lib/drivers/
ILIST           = diagex

CFLAGS=-D_KERNEL \
	-DTDEBUG 
	
CARGS=-qlist \
	-qsource 

diagex_ENTRYPOINT = diag_open

diagex_EXPORTS = -bE:diagex.exp

diagex_OFILES = diagex_pin.o diagex_load.o

IMPORTS = ${KERNEXT_IMPORTS}

LIBS = ${KERNEXT_LIBS} \
	-lc

.include <${RULES_MK}>

root/bos.diag.rte.trc.S: bos.diag.rte.trc.S
		-${MKDIR} root
		${CP} ${bos.diag.rte.trc.S:P} root/bos.diag.rte.trc.S
