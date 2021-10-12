# @(#)21        1.4  src/bldenv/mk/osf.aix.script.mk, ade_build, bos412, GOLDA411a 5/3/94 12:07:03
#
#   ORIGINS: 71
#
#                    SOURCE MATERIALS
#
#
# 
# (c) Copyright 1991, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
# 
#
#
# OSF/1 1.1 Snapshot 2

.if !defined(_OSF_SCRIPT_MK_)
_OSF_SCRIPT_MK_=

#
#  Definitions for rules using sed
#
_N_A_S_F_=THIS IS NOT A SOURCE FILE - DO NOT EDIT

#
#  Default single suffix compilation rules
#
.csh:	${MAKEFILE_DEPENDENCY}
	${SED} -e '1s;^#!;&;' -e t\
	       -e 's;^#\(.*\)\@SOURCEWARNING\@;\1${_N_A_S_F_};' -e t\
	       ${${.TARGET}_SED_OPTIONS:U${SED_OPTIONS}}\
	       -e '/^[ 	]*#/d' ${.IMPSRC} >${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}
	${CHMOD} +x ${.TARGET}

.sh:	${MAKEFILE_DEPENDENCY}
	${CP} ${.IMPSRC} ${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}
	${CHMOD} +x ${.TARGET}

.S:	${MAKEFILE_DEPENDENCY}
	@${RM} ${_RMFLAGS_} ${.TARGET}
	${SED} "/^#/d" <${.IMPSRC} >${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE}; }
	${CHMOD} +x ${.TARGET}

.shN:	${MAKEFILE_DEPENDENCY}
	@${RM} ${_RMFLAGS_} ${.TARGET}
	${BRAND} ${BRANDDICT} <${.IMPSRC} >${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE}; }
	${CHMOD} +x ${.TARGET}

.SN:	${MAKEFILE_DEPENDENCY}
	@${RM} ${_RMFLAGS_} ${.TARGET}
	${BRAND} ${BRANDDICT} <${.IMPSRC} >${.TARGET}.X || { ${RM} -f ${.TARGET}; ${FALSE}; }
	${SED} "/^#/d" <${.TARGET}.X >${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE}; }
	${CHMOD} +x ${.TARGET}

.endif
