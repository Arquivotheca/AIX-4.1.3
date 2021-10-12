# @(#)17        1.3  src/bos/diag/tu/370pca/Makefile, tu_370pca, bos41J, 9523C_all 6/8/95 14:31:19
# COMPONENT_NAME: tu_370pca
#
# FUNCTIONS: (Makefile) Makefile for 370PCA TU's
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1992, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


#---------------------------------------------------------------------------#
#                                                                           #
#     This Makefile builds the library lib370pcatu.a for the DA application #
#                                                                           #
#---------------------------------------------------------------------------#

LIBRARIES       =   lib370pcatu.a

EXPLIB_TARGETS  =   export_lib370pcatu.a
EXPDIR          =   /usr/lib/

CFLAGS          = -DAIX -DDIAGS

OFILES          = crc_gen.o \
	          hardware.o \
	          misc.o \
	          pscatu.o \
	          tu_mcip.o \
	          tu_fifo.o \
	          tu_int.o \
	          tu_dma.o \
	          castart.o \
	          tu_sram.o

ILIST           = fe92d.00.01
fe92d.00.01_IDIR = /usr/lib/microcode/

.include <${RULES_MK}>
