# @(#)13	1.4  src/bos/diag/util/ufd/Makefile, dsaufd, bos411, 9428A410j 12/5/93 21:06:47
#
#   COMPONENT_NAME: DSAUFD
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


PROGRAMS	= ufd

MSGHDRS		= ufd_msg.h
CATFILES 	= ufd.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc

ILIST		= ${PROGRAMS} ufd.cat
IDIR		= /usr/lpp/diagnostics/bin/
ufd.cat_IDIR = /usr/lpp/diagnostics/catalog/default/
ufd_OFILES	= fdsatest.o asl_rtn.o fdsamain.o fdsamenu.o

.include <${RULES_MK}>
