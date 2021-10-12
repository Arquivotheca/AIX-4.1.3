# @(#)67        1.1  src/bldenv/uconvdef/Makefile, ade_build, bos412, GOLDA411a 12/16/93 18:18:36
#
#   COMPONENT_NAME: ADE_BUILD
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
#

VPATH		= ${MAKETOP}/bos/usr/bin/uconvdef:${MAKETOP}/bos/usr/include:${MAKETOP}/bos/kernel/sys

PROGRAMS	= uconvdef
MSGHDRS		= uconvdef_msg.h
CATFILES	= uconvdef.cat
HFILES		= ${MSGHDRS}

INCLUDES	= uc_convP.h types.h nl_types.h
EXPDIR		= /usr/include/
types.h_EXPDIR	= /usr/include/sys/

ILIST		= uconvdef
IDIR		= /usr/bin/

.include <${RULES_MK}>
