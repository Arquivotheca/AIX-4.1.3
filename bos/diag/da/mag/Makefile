#
#
#
#   ORIGINS: 27
#
#
#

ILIST           = dmag dmag.cat
IDIR		= /usr/lpp/diagnostics/da/
dmag.cat_IDIR = /usr/lpp/diagnostics/catalog/default/

OTHERS =
OTHERS += dmag.cat
dmag.cat: dmag.cat.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS += dmag
dmag: dmag.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
