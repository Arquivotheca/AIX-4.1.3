# @(#)99	1.5  src/bos/usr/ccs/lib/libm/Makefile, libm, bos411, 9428A410j 9/21/93 11:52:49
#
#   COMPONENT_NAME: LIBM
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1988,1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#  WARNING - When this Makefile is changed, you must make the
#            corresponding change to the Makefile in the profiled
#            directory


EXPLIB_SUBDIRS	= profiled
SUBDIRS		= profiled

VPATH		=${TARGET_MACHINE}

LIBRARIES	=libm.a
MSGHDRS		=
CATFILES	=
HFILES		= ${MSGHDRS}

EXPLIB_TARGETS	= export_libm.a

ILIST		=libm.a
IDIR		= /usr/ccs/lib/

INCFLAGS= -I${TARGET_MACHINE}
CFLAGS=	-qnofold -qdebug=nglobshd

OFILES=	\
	acos.o acosh.o asin.o asinh.o atan.o atan2.o atanh.o \
	cabs.o cbrt.o class.o copysign.o cos.o cosh.o drem.o \
	erf.o exp.o exp__E.o finite.o floor.o isnan.o lgamma.o	\
	ilogb.o	j0.o j1.o jn.o log.o log10.o log1p.o logb.o	\
	log__L.o nextaft.o pow.o scalb.o sin.o sinh.o sqrt.o	\
	tan.o tanh.o tantab.o unord.o acosF.o asinF.o atan2F.o	\
	atanF.o cosF.o coshF.o erfF.o expF.o floorF.o lgammaF.o	\
	log10F.o logF.o powF.o sinF.o sinhF.o sqrtF.o tanF.o	\
	tanhF.o	rsqrt.o rsqrtF.o __set_errno128.o	\
	${${TARGET_MACHINE}_OFILES}

.include <${TARGET_MACHINE}/machdep.mk>

# The following are actually defined in the POWER/machdep.mk
# POWER_OFILES variable, but are included here and in
# ./profiled/Makefile to make profiling work right.

ceill.o_CFLAGS += -qldbl128 -D__LONGDOUBLE128
fabsl.o_CFLAGS += -qldbl128 -D__LONGDOUBLE128
floorl.o_CFLAGS += -qldbl128 -D__LONGDOUBLE128
fmodl.o_CFLAGS += -qldbl128 -D__LONGDOUBLE128

.include <${RULES_MK}>
