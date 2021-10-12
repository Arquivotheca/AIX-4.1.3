#
#
#
#   ORIGINS: 27
#
#
#

IDIR                    = /usr/lib/methods/
ILIST                   = cfgdac


OTHERS=

OTHERS+=cfgdac
cfgdac:cfgdac.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
