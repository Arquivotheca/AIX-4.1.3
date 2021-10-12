# @(#)77	1.1  src/bos/usr/bin/bterm/Makefile, bos, bos410 8/26/93 13:34:45
#
#   COMPONENT_NAME: LIBBIDI
#
#   FUNCTIONS: none
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

PROGRAMS	= bterm

ILIST		= ${PROGRAMS}
 
IDIR		= /usr/bin/

OFILES		= main.o ttyio.o ptyio.o screen.o refresh.o \
                  bidifunc.o parse.o kbd.o push.o debug.o \
                  defaults.o status.o help.o 3151func.o \
                  vt220func.o hft_func.o resources.o
 
CFLAGS		+= ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURSESLIBFLAG}

LIBS			= -lc -li18n -lcurses 

.include <${RULES_MK}>



