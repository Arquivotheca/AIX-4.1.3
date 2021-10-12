#
#
#
#   ORIGINS: 27
#
#
#

IDIR                    = /usr/lib/ras/
ILIST                   = notifymethscarray


OTHERS=

OTHERS+=notifymethscarray
notifymethscarray:notifymethscarray.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
