# @(#)72        1.9  src/bos/usr/include/diag/Makefile, cmddiag, bos41J, 9514A_all 4/4/95 16:17:50
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 10,27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BINARIES	= yes

IDIR		= /usr/include/diag/

DATAFILES	= \
	diagodm.h	\
	atu.h	\
	bit_def.h	\
	da.h	\
	diag.h	\
	diag_define.h \
	diag_exit.h	\
	diago.h	\
	tm_input.h	\
	tmdefs.h

# Header files not in inslist should be listed below
# but not in the DATAFILES.

INCLUDES	= ${DATAFILES} class_def.h assign_bit.h diagvpd.h \
		  modid.h har2_atu.h ktmcb.h scsi_atu.h ttycb.h \
		  diagresid.h scsd.h dascsi.h da_cmn.h da_rc.h \
		  diag_trace.h

ILIST		= ${DATAFILES}

.include <${RULES_MK}>

