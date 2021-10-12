# @(#)06	1.3  src/bos/usr/bin/common/Makefile, cmdlft, bos411, 9428A410j 4/8/94 13:25:47

# COMPONENT_NAME:
#
# FUNCTIONS:
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

INCLUDES    = lftcmds_msg.h lftcmds_cat.h lftcmds.h
EXPDIR      = /usr/include/
MSGHDRS		= lftcmds_msg.h 
CATFILES	= lftcmds.cat
HFILES		= ${MSGHDRS}
OTHERS		= lftcmds_cat.h

.include <${RULES_MK}>

lftcmds_cat.h:  lftcmds.prolog lftcmds.msg
	${SED} -e 's/^\$$.*$$//' < ${lftcmds.msg:P} > cmdlft.out1
	${SED} -e '/^[ ^I]*$$/d' cmdlft.out1 > cmdlft.out2
	${CAT} ${lftcmds.prolog:P} > lftcmds_cat.h
	${CAT} cmdlft.out2 | ${SED} -e 's/^\([^   \\]\)\(.*\)/#define M_\1\2/' >> lftcmds_cat.h
	${RM} -f cmdlft.out1
	${RM} -f cmdlft.out2

