EXPLIB_TARGETS	=	export_libtu_ruby.a
EXPDIR		=	/usr/lib/


OTHERS=

OTHERS+=libtu_ruby.a
libtu_ruby.a:libtu_ruby.a.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
