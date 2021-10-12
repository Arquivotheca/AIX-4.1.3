# @(#)25        1.5  src/bos/diag/tu/wga/Makefile, tu_wga, bos411, 9428A410j 1/4/94 09:38:36
#
#   COMPONENT_NAME: TU_WGA
#
#   FUNCTIONS: (Makefile) Makefile for WGA DA TU's
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
##########################################################################


#---------------------------------------------------------------------------#
#                                                                           #
#     This Makefile builds the library libwgatu.a for the DA/SA application #
#                                                                           #
#---------------------------------------------------------------------------#

LIBRARIES       =   libwgatu.a

INCFLAGS        =   -I${MAKETOP}/bos/kernext/disp/wga/inc

EXPLIB_TARGETS  =   export_libwgatu.a
EXPDIR          =   /usr/lib/

CFLAGS          =   -DWORK_AROUND

OFILES          = at.o colorbartu.o copystr.o crosstu.o cursortu.o  \
	          drawcmds.o exectu.o fastcpytu.o inttu.o meminfo.o  \
	          moveit.o palettetu.o pixelcmdstu.o regtu.o rgbtu.o \
	          Sigmom.o sqrboxtu.o strtesttu.o videoromtu.o vpdtu.o \
	          vramtu.o wgamisc.o

LIBS            =   libc.a libm.a

.include <${RULES_MK}>
