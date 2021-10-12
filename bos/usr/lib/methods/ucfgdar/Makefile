#
#
#
#   ORIGINS: 27
#
#
#

IDIR                    = /usr/lib/methods/
ILIST                   = ucfgdar


OTHERS=

OTHERS+=ucfgdar
ucfgdar:ucfgdar.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
