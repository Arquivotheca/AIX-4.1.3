SUBDIRS			=	shlib
ILIST			= ksh alias ksh.cat
IDIR			= /usr/bin/


OTHERS=

OTHERS+=ksh
ksh:ksh.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=alias
alias:alias.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=ksh.cat
ksh.cat:ksh.cat.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
