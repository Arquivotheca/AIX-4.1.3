# @(#)73	1.2  src/bos/diag/tu/baud/Makefile, tu_baud, bos411, 9439B411a 9/29/94 10:54:37
#
#   COMPONENT_NAME: (tu_baud) Ultimedia Audio Adapter Test Units
#
#   FUNCTIONS: Makefile
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

VPATH = ${MAKETOP}/bos/kernext/exp

LIBRARIES = libtu_baud.a

EXPLIB_TARGETS	= export_libtu_baud.a
EXPDIR		= /usr/lib/

#IMPORTS = -bI:rcm.exp -bI:diagex.exp 

CFLAGS = \
	${RS2_601} \
	-D_POWER \
	-DLFT_ACCESS

OFILES = \
	baud_exectu.o \
	baudtu.o \
	vpd_check.o \
	baudutil.o

.include <${RULES_MK}>
