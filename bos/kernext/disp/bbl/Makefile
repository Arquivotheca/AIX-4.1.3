ILIST	 	= bbldd bblddpin
IDIR		= /usr/lib/drivers/

OTHERS=

OTHERS+=bblddpin
bblddpin:bblddpin.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=bbldd
bbldd:bbldd.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
