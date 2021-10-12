EXPLIB_TARGETS = export_libsgatu.a
EXPDIR		=	/usr/lib/

OTHERS=

OTHERS+=libsgatu.a
libsgatu.a:libsgatu.a.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
