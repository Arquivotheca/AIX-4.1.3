#
#
#
#   ORIGINS: 27
#
#
#

IDIR                    = /usr/bin/
ILIST                   = raidmgr sm_ibm7135.cat

sm_ibm7135.cat_IDIR	= /usr/lib/methods/


OTHERS=

OTHERS+=raidmgr
raidmgr:raidmgr.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=sm_ibm7135.cat
sm_ibm7135.cat:sm_ibm7135.cat.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
