# @(#)78        1.2  src/bos/diag/tu/corv/Makefile, tu_corv, bos411, 9428A410j 2/1/94 15:39:24
#
#   COMPONENT_NAME: TU_CORV
#
#   FUNCTIONS: 
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

#------------------------------------------------------------------------#
#                                                                        #
#     This Makefile builds the library libcorvtu.a for the DA application #
#                                                                        #
#------------------------------------------------------------------------#


SUBDIRS         = slih

EXPINC_SUBDIRS  = slih
EXPLIB_SUBDIRS  = slih

CFLAGS	= -D CORVETTE -D DIAGNOSTICS

LIBRARIES       =   libcorvtu.a

EXPLIB_TARGETS  =   export_libcorvtu.a
EXPDIR          =   /usr/lib/

OFILES		= Adapters.o CmdBld.o ScsiBld.o \
	  commands.o corv_atu.o corvette.o getcorvette.o tsb.o

.include <${RULES_MK}>
