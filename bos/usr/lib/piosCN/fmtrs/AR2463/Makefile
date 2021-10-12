#
#   ORIGINS: 27
#

ILIST = \
	piofar2463ehsc \
	piofar2463essc

IDIR 	= /usr/lib/lpd/pio/fmtrs/


OTHERS =
OTHERS += piofar2463ehsc
piofar2463ehsc: piofar2463ehsc.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS += piofar2463essc
piofar2463essc: piofar2463essc.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
