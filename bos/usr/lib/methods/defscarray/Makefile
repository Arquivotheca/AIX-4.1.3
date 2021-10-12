#
#
#
#   ORIGINS: 27
#
#
#

IDIR                    = /usr/lib/methods/
ILIST                   = defscarray


OTHERS=

OTHERS+=defscarray
defscarray:defscarray.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
