# @(#)39        1.19  src/bos/kernel/ios/Makefile, sysios, bos41J, 9523AA_all 6/7/95 12:37:05
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# LEVEL 1,  5 Years Bull Confidential Information
#

.include <${MAKETOP}bos/kernel/Kernel.mk>

VPATH		=${TARGET_MACHINE}

INCFLAGS	=-I${TARGET_MACHINE}

CFLAGS		+= -D_GOLD

PAGED_OFILES	=	\
	bio_paged.o	\
	devsw_paged.o	\
	poll.o		\
	select.o	\
	sysconfig.o	\
	uphysio_paged.o \
	${${TARGET_MACHINE}_PAGED_OFILES}

PINNED_OFILES	=	\
	bio_pin.o	\
	clist.o         \
	conf.o		\
	devsw_pin.o	\
	intr.o		\
	iost.o		\
	mpc.o		\
	selnotify.o	\
	selpoll.o	\
	uio.o		\
	uphysio_pin.o	\
	pm_kernel.o	\
	${${TARGET_MACHINE}_PINNED_OFILES}

INIT_OFILES	=	\
	binit.o		\
	cinit.o		\
	devsw_init.o	\
	intr_init.o	\
	iost_init.o	\
	selpollinit.o	\
	uphysinit.o	\
	uio_init.o	\
	pm_kernel_init.o \
	${${TARGET_MACHINE}_INIT_OFILES}

.include <${TARGET_MACHINE}/machdep.mk>

.include <${RULES_MK}>

# Assignment of SED_SUBST added by Michael Winestock (MJW) 04/02/93.  Define
# SED substitution for replacing __default_build__ with the value of
# $default_build.
#
SED_SUBST	="`${ECHO} ${default_build} | ${CUT} -d\".\" -f2`"

# Following modified by Richard Letsinger 5 Jan 94.  Don't need to brand
# anymore.  Do want to continue to put the build in uname.
#
# Rule added by Michael Winestock (MJW) 04/02/93.  Produce the conf.c file by:
#
# Old (MJW):
#   1) Substituting __default_build__ with $default_build.  This will
#      produce a conf.cN in the target directory.
#   2) Branding the conf.cN file in the target directory.  This produces the
#      conf.c file.
#   3) Remove the conf.cN file in the target directory.
# New (RLL):
#   1) Substituting __default_build__ with $default_build.  This will
#      produce a temporary conf.c in the target directory.
#   2) Compile the temporary conf.c to produce a conf.o.
#   3) Remove the temporary conf.c.
#
conf.o : conf.c ${MAKEFILE_DEPENDENCY}
	@${RM} ${_RMFLAGS_} ${.TARGET}
	${SED} -e s/__default_build__/${SED_SUBST}/ ${conf.c:P} > conf.c
	${_CC_} -c ${_CCFLAGS_} conf.c
	@${RM} ${_RMFLAGS_} conf.c
