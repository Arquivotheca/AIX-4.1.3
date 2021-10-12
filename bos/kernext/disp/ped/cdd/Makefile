ILIST	 	= mid_cdd mid_cdd_pin
IDIR		= /usr/lib/drivers/

OTHERS=

OTHERS+=mid_cdd_pin
mid_cdd_pin:mid_cdd_pin.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=mid_cdd
mid_cdd:mid_cdd.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
