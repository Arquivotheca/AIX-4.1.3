ILIST		= druby druby.cat
IDIR		= /usr/lpp/diagnostics/da/
druby.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/


OTHERS=

OTHERS+=druby
druby:druby.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=druby.cat
druby.cat:druby.cat.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
