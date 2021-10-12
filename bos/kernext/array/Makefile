#
#
#	ORIGINS:	27
#
#
#

ILIST	= scarray scarraypin
IDIR	= /usr/lib/drivers/

OTHERS=
OTHERS+=scarray
scarray:scarray.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=scarraypin
scarraypin:scarraypin.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
