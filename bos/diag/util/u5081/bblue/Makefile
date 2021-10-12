ILIST		= ubblue ubblue.cat
IDIR		= /usr/lpp/diagnostics/bin/
ubblue.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

OTHERS=

OTHERS+=ubblue
ubblue:ubblue.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=ubblue.cat
ubblue.cat:ubblue.cat.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
