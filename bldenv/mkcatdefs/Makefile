# @(#)51        1.1  src/bldenv/mkcatdefs/Makefile, ade_build, bos412, GOLDA411a 2/17/93 08:53:26
#
#   COMPONENT_NAME: BOSBUILD
#
#   FUNCTIONS:
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

VPATH		= ${MAKETOP}/bos/usr/bin/mkcatdefs:${MAKETOP}/bos/usr/bin/gencat

PROGRAMS	= mkcatdefs
MSGHDRS		= msgfac_msg.h
CATFILES	= msgfac.cat
HFILES		= ${MSGHDRS}

ILIST		= mkcatdefs
IDIR		= /usr/bin/

INCFLAGS	=-I${MAKETOP}/bos/usr/bin/gencat

OFILES		= mkcatdefs.o symtab.o catio.o

.include <${RULES_MK}>
