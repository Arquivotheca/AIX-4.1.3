#
#
#
#   ORIGINS: 27
#
#
#

ILIST		= abcnewphrase Abcnewphrase
IDIR		= /usr/lpp/cnls/bin/
Abcnewphrase_IDIR	= /usr/lpp/X11/lib/X11/zh_CN/app-defaults/


OTHERS =
OTHERS += abcnewphrase
abcnewphrase: abcnewphrase.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
