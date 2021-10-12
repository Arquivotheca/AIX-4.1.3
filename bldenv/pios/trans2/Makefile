# @(#)87	1.4  src/bldenv/pios/trans2/Makefile, cmdpios, bos412, GOLDA411a 1/18/94 10:01:27
# COMPONENT_NAME: CMDPIOS
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH 		= ${MAKETOP}bos/usr/lib/pios/trans2

OTHERS		= r420x.437 rdp2000 rd630 rhplj.dn rhplj.ecma rhplj.ibmus \
		  rhplj.roman8 r4216.roman8 ribm.437 ribm.850 ribm.851 \
		  ribm.852 ribm.853 ribm.855 ribm.857 ribm.860 ribm.862 \
		  ribm.863 ribm.864 ribm.865 ribm.869 ribm.1046 r3812.p0 \
		  r3812.p1 r3812.p2 r3816.p1 r3816.p2 rp9012 rascii r420x.850

INCFLAGS	= -I${MAKETOP}bos/usr/lib/pios/trans2
ILIST		= ${OTHERS}
IDIR		= /usr/lib/lpd/pio/trans2/

.include <${RULES_MK}>

${OTHERS} : $${.TARGET:S|^r||}.c
	${RM} ${_RMFLAGS_} ${.TARGET}
	${CC} -o ${.TARGET} ${_CCFLAGS_} ${_LDFLAGS_} ${.ALLSRC} 
	${MD} ${_MDFLAGS_} .
