# @(#)62	1.2  src/bos/usr/lpp/Unicode/wim/Makefile, cfgnls, bos411, 9428A410j 2/3/94 16:07:19
#
#   COMPONENT_NAME: CFGNLS
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

PROGRAMS	= UNIVERSAL.im
MSGHDRS		=
CATFILES	=
HFILES		= ${MSGHDRS}

${PROGRAMS}_ENTRYPOINT = WIMInitialize

OFILES		= wim.o 

ILIST		= ${PROGRAMS} UNIVERSAL.imcfg
IDIR		= /usr/lpp/Unicode/loc/

INCFLAGS	= -I${MAKETOP}/bos/usr/ccs/lib/libim -I${MAKETOP}/bos/usr/include
LIBS		= -lIM -liconv

.include <${RULES_MK}>
