#
#
#
#   ORIGINS: 27
#
#
#

ILIST		= cndictutil CNdictutil
IDIR		= /usr/lpp/cnls/bin/
CNdictutil_IDIR	= /usr/lpp/X11/lib/X11/zh_CN/app-defaults/


OTHERS =
OTHERS += cndictutil
cndictutil: cndictutil.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
