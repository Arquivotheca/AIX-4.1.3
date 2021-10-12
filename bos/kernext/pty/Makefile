#
# @BULL_COPYRIGHT@
#
# HISTORY
# $Log: Makefile,v $
# Revision 1.1.12.3  1994/03/14  09:59:25  ccb
# 	Updated with Austin screen team version.
# 	Correction for defect #126854.
# 	[1994/03/14  09:56:54  ccb]
#
# Revision 1.1.12.2  1994/02/21  15:14:09  ccb
# 	Added modifications for ttydbg,
# 	Added "COPYRIGHT" header.
# 	[1994/02/21  15:13:42  ccb]
# 
# $EndLog$
#
# END-BULL-ODE

# @(#)24 1.8 src/bos/kernext/pty/Makefile, sysxpty, bos41J, 9517B_all 4/21/95 07:40:11
#
# COMPONENT_NAME: (sysxpty)
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
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
# LEVEL 1, 5 Years Bull Confidential Information
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

KERNEL_EXT	= ptydd 

IDIR		= /usr/lib/drivers/
ILIST		= ${KERNEL_EXT}
LIBS            = ${KERNEXT_LIBS}
VPATH   	=${MAKETOP}bos/kernel/exp:${MAKETOP}bos/kernel/exp/${TARGET_MACHINE}:${MAKETOP}bos/kernext/exp:../tty:
OFILES		= spty.o \
		  spty_db.o 


LOCALCFLAGS	= -O
OPT_LEVEL	= 

KERNEXT_IMPORTS	= sysoverlay.exp kernex.exp syscalls.exp -bI:pse.exp -bI:dead.exp -bI:pse_int.exp -bI:dbtty.exp -bI:loadfs.exp

ptydd_LIBS		= libcsys.a
ptydd_ENTRYPOINT	= ptsm_config
ptydd_IMPORTS		= -bI:ldterm.exp -bI:ttydbg.exp
ptydd_OFILES		= spty.o spty_db.o 

.include <${RULES_MK}>

