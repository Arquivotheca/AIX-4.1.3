ILIST		= usga
IDIR		= /usr/lpp/diagnostics/bin/

OTHERS=

OTHERS+=usga
usga:usga.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
