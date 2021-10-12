SUBDIRS		=	interrupt
EXPINC_SUBDIRS	=	interrupt
EXPLIB_SUBDIRS	=	interrupt

EXPLIB_TARGETS	=	export_libgiotu.a
EXPDIR		=	/usr/lib/


OTHERS=

OTHERS+=libgiotu.a
libgiotu.a:libgiotu.a.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
