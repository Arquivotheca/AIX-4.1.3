#
#
#
# ORIGINS: 27
#
#
#
#

IDIR    	   = /usr/lpp/Unicode/tty/
ILIST    	   = uc_utf lc_utf UTF-8


OTHERS =
OTHERS += uc_utf
uc_utf: uc_utf.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS += lc_utf
lc_utf: lc_utf.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
