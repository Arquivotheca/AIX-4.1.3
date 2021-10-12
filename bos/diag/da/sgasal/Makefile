ILIST		= dsga dsga.cat
IDIR		= /usr/lpp/diagnostics/da/
dsga.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/


OTHERS=

OTHERS+=dsga
dsga:dsga.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=dsga.cat
dsga.cat:dsga.cat.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
