ILIST		= hiadd
IDIR		= /usr/lib/drivers/

OTHERS=

OTHERS+=hiadd
hiadd:hiadd.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
