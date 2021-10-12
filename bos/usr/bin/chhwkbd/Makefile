# @(#)75        1.2  src/bos/usr/bin/chhwkbd/Makefile, cmdinputdd, bos411, 9428A410j 10/24/93 15:24:28
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
PROGRAMS    = chhwkbd
MSGHDRS     = chhwkbd_msg.h
CATFILES    = chhwkbd.cat
HFILES      = ${MSGHDRS}

ILIST       = chhwkbd
IDIR        = /usr/bin/
LIBS        = -lodm -lcfg

OTHERS	= chhwkbd_msgdft.h

.include <${RULES_MK}>

chhwkbd_msgdft.h:  chhwkbd.msg
	${SED} -e 's/^\$$.*$$//' < ${chhwkbd.msg:P} > chhwkbd.out1
	${SED} -e '/^[ ^I]*$$/d' chhwkbd.out1 > chhwkbd.out2
	${CAT} chhwkbd.out2 | ${SED} -e 's/^\([^   \\]\)\(.*\)/#define M_\1\2/' > chhwkbd_msgdft.h
	${RM} -f chhwkbd.out1
	${RM} -f chhwkbd.out2
