#
#
#   ORIGINS: 27
#
#


ILIST = \
	pioflq1600kessc \
	pioflq1600kehsc \
	pioflq1600kUTF-8

IDIR 	= /usr/lib/lpd/pio/fmtrs/

OTHERS =
OTHERS += pioflq1600kessc
pioflq1600kessc: pioflq1600kessc.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS += pioflq1600kehsc
pioflq1600kehsc: pioflq1600kehsc.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS += pioflq1600kUTF-8
pioflq1600kUTF-8: pioflq1600kUTF-8.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}


.include <${RULES_MK}>
