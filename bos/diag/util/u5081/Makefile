# @(#)07	1.14  src/bos/diag/util/u5081/Makefile, dsau5081, bos41B, 9505A 1/23/95 17:13:37
#
#   COMPONENT_NAME: DSAU5081
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


PROGRAMS	= u5081

# removing subdirs for all but sga and wga. Others must be added as they are
# ready.

# SUBDIRS		= sky ped lega lega2 swmono wga sga gem

SUBDIRS		= wga sga bblue sky ped lega2 swmono unep gem

MSGHDRS		= u5081_msg.h
CATFILES 	= u5081.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc

INCFLAGS	= -I${MAKETOP}/bos/usr/include

ILIST		= ${PROGRAMS} u5081.cat
IDIR		= /usr/lpp/diagnostics/bin/
u5081.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/
u5081_OFILES	= u5081.o

.include <${RULES_MK}>
