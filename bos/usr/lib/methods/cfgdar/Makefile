#
#
#
#   ORIGINS: 27
#
#
#

IDIR                    = /usr/lib/methods/
ILIST                   = cfgdar


OTHERS=

OTHERS+=cfgdar
cfgdar:cfgdar.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
