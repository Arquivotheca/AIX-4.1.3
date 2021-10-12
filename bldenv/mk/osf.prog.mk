# @(#)19        1.2  src/bldenv/mk/osf.prog.mk, ade_build, bos412, GOLDA411a 5/3/94 12:15:54
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

.if !defined(_OSF_PROG_MK_)
_OSF_PROG_MK_=

#
#  Build rules
#
.if defined(PROGRAMS)

${PROGRAMS}: $${_OFILES_} ${MAKEFILE_DEPENDENCY}
	${_CC_} ${_LDFLAGS_} -o ${.TARGET}.X ${_OFILES_} ${_LIBS_}
	${MV} ${.TARGET}.X ${.TARGET}

.endif

.endif
