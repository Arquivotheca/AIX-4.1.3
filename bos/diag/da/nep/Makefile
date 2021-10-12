ILIST           = dnep dnep.cat
IDIR            = /usr/lpp/diagnostics/da/
dnep.cat_IDIR = /usr/lpp/diagnostics/catalog/default/


OTHERS=

OTHERS+=dnep
dnep:dnep.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=dnep.cat
dnep.cat:dnep.cat.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
