# @(#)23        1.3  src/bos/usr/lib/nls/loc/sim/Makefile, cmdims, bos411, 9428A410j 2/2/94 21:12:27
#
# COMPONENT_NAME: cmdims
#
# FUNCTIONS: SBCS Input method (Dynamic Composing IM (DIM))
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1988, 1991, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= sbcs.im
MSGHDRS		=
CATFILES	=
HFILES		= ${MSGHDRS}

${PROGRAMS}_ENTRYPOINT = DIMInitialize

OFILES		= dim.o OpenCompose.o

ILIST		= ${PROGRAMS}
IDIR		= /usr/lib/nls/loc/

INCFLAGS	= -I${MAKETOP}/bos/usr/ccs/lib/libim -I${MAKETOP}/bos/usr/include
LIBS		= -lIM

.include <${RULES_MK}>
