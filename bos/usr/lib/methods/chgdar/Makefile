#
#
#
#   ORIGINS: 27
#
#
#

IDIR                    = /usr/lib/methods/
ILIST                   = chgdar


OTHERS=

OTHERS+=chgdar
chgdar:chgdar.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
