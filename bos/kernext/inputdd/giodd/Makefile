ILIST             = giodd
IDIR              = /usr/lib/drivers/


OTHERS=

OTHERS+=giodd
giodd:giodd.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
