ILIST		= dbblue dbblue.cat
IDIR		= /usr/lpp/diagnostics/da/
dbblue.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

OTHERS=

OTHERS+=dbblue
dbblue:dbblue.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=dbblue.cat
dbblue.cat:dbblue.cat.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
