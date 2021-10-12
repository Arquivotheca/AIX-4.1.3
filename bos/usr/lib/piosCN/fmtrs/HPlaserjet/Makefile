#
#
#   ORIGINS: 27
#
#
#

ILIST = \
	pioflaserjetxessc \
	pioflaserjetxUTF-8

IDIR 	= /usr/lib/lpd/pio/fmtrs/

OTHERS =
OTHERS += pioflaserjetxessc
pioflaserjetxessc: pioflaserjetxessc.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS += pioflaserjetxUTF-8
pioflaserjetxUTF-8: pioflaserjetxUTF-8.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}


.include <${RULES_MK}>
