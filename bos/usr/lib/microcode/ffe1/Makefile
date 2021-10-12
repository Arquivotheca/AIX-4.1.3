#
#
#
#   ORIGINS: 27
#
#
#

IDIR                    = /usr/lib/microcode/
ILIST                   = cxbios.02.09 cxfep.02.09 isacxbios.02.09 isacxfep.02.09


OTHERS=

OTHERS+=isacxbios.02.09
isacxbios.02.09:isacxbios.02.09.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=isacxfep.02.09
isacxfep.02.09:isacxfep.02.09.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
