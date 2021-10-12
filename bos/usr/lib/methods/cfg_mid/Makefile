ILIST		= cfg_mid
IDIR		= /usr/lib/methods/

OTHERS=

OTHERS+=cfg_mid
cfg_mid:cfg_mid.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
