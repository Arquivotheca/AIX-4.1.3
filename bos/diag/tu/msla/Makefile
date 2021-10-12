# @(#)46	1.2  src/bos/diag/tu/msla/Makefile, tu_msla, bos411, 9428A410j 1/7/94 13:15:48
#
#   COMPONENT_NAME: TU_MSLA
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


#-------------------------------------------------------------------------#
#                                                                         #
#     This Makefile builds the library libmslatu.a for the DA application #
#                                                                         #
#-------------------------------------------------------------------------#

LIBRARIES       =   libmslatu.a

EXPLIB_TARGETS  =   export_libmslatu.a
EXPDIR          =   /usr/lib/

OFILES		= 	mslaexectu.o mslapostst.o mslamemtst.o mslaregtst.o \
			mslavpdtst.o msla19test.o msla20test.o mslainttst.o \
			msladma.o atox.o busopen.o loadmsla.o mkerr.o \
			mslaiorw.o util.o

.include <${RULES_MK}>
