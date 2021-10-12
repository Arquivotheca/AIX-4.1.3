# @(#)39       1.1  src/bldenv/ctags/Makefile, bldprocess, bos412, GOLDA411a 4/26/93 07:59:43
#
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1992
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/usr/bin/ex

PROGRAMS	= ctags
MSGHDRS		= ex_msg.h

ILIST		= ctags
IDIR		= /usr/bin/

TRACEDEFS	= -DTRACE -DUNFOTRACE -DADEBUG -DMDEBUG -DFDEBUG -DIDEBUG
MYDEFS		= -DMYSIG -DSUSPTRACE -DMYALLOC -UMYDEBUG -URDEBUG ${TRACEDEFS}
UNDEFS		= -DSTDIO -UUCBV7 -UCVISUAL -UV6 -UVFORK ${MYDEFS}

CFLAGS		= -UFLOCKFILE -D_SECURITY -DCHDIR -DLISPCODE \
		  -DSINGLE -DTABS=8 -DUSG -DVMUNIX ${UNDEFS}

.include <${RULES_MK}>

ctags.o:	ex_msg.h
