EXPLIB_TARGETS	=	export_libtu_nep.a
EXPDIR		=	/usr/lib/


OTHERS=

OTHERS+=libtu_nep.a
libtu_nep.a:libtu_nep.a.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
