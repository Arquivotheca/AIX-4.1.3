# @(#)12	1.2  src/bos/diag/util/udiskenh/Makefile, dsaudiskenh, bos411, 9435A411a 8/22/94 15:41:15
#
#   COMPONENT_NAME: DSAUDISKENH
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1993.
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


PROGRAMS	= udiskmnt udmedit

MSGHDRS		= udiskmnt_msg.h udmedit_msg.h
CATFILES 	= udiskmnt.cat udmedit.cat
HFILES		= ${MSGHDRS} 

LIBS		= -lasl -ldiag -lodm -lcfg -lc -llvm -lcur

ILIST		= ${PROGRAMS} udiskmnt.cat udmedit.cat
IDIR		= /usr/lpp/diagnostics/bin/
udiskmnt.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/
udmedit.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/
udiskmnt_OFILES		= udiskmnt.o udmutil.o udmcopy.o udmbbdir.o udmdisp.o 
udmedit_OFILES          = udmedit.o udmhexit.o udmmisc.o udmwindows.o

.include <${RULES_MK}>
