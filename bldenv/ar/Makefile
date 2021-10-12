# @(#)84        1.1  src/bldenv/ar/Makefile, ade_build, bos412, GOLDA411a 2/17/93 07:22:32
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

VPATH			= ${MAKETOP}/bos/usr/ccs/bin/ar

PROGRAMS		= ar
MSGHDRS                 = ar_msg.h 
CATFILES                = ar.cat

ILIST			= ar
IDIR			= /usr/bin/

CFLAGS			= -DXCOFF -DTOBEY_BUG -DLIBCIO_FIX -D_STD_ARG 
HFILES			= ${MSGHDRS} 

OFILES			= ar.o 

LIBS			= -lld


.include <${RULES_MK}>
