# @(#)53	1.2  src/bos/usr/bin/errlg/samples/Makefile, cmderrlg, bos411, 9428A410j 5/21/94 09:14:06
#
# COMPONENT_NAME:  (CMDERRLG)
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= getvfsname
SCRIPTS		= corepath
SAMPLES		= getvfsname.c README ${SCRIPTS}

ILIST		= ${PROGRAMS} ${SAMPLES}
IDIR		= /usr/samples/findcore/

.include <${RULES_MK}>

getvfsname.c: getvfsname.c.S
	${CP} ${getvfsname.c.S:P} ${.TARGET}

README:	README.S
	${CP} ${README.S:P} ${.TARGET}
