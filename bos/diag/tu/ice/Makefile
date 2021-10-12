# @(#)92	1.1  src/htx/usr/lpp/htx/lib/hga/Makefile, tu_hga, htx410 6/2/94 11:37:36
#
#   COMPONENT_NAME: tu_hga
#
#   FUNCTIONS: none
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
###########################################################################

VPATH = ${MAKETOP}/rspc/kernext/disp/pcihga

LIBRARIES	= libtu_ice.a

INCFLAGS = -I${MAKETOP}/rspc/kernext/disp/pcihga
LIBS		= 

EXPLIB_TARGETS  =   export_libtu_ice.a
EXPDIR          =   /usr/lib/

#OPT_LEVEL =
OFILES		 = \
        exectu.o \
	hga_open.o \
	palette.o  \
	bt485dac.o \
	vramtest.o \
	font8x16.o \
	util.o


ILIST		= 
IDIR		=

.include <${RULES_MK}>

