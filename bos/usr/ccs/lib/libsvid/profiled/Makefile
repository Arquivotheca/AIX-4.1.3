# @(#)58        1.5  src/bos/usr/ccs/lib/libsvid/profiled/Makefile, libsvid, bos411, 9428A410j 5/4/94 11:27:46
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= 	\
..:../${TARGET_MACHINE}:${MAKETOP}/bos/usr/ccs/lib/libm:${MAKETOP}/bos/usr/ccs/lib/libm/${TARGET_MACHINE}

LIBRARIES	= libmsaa.a
MSGHDRS         =
CATFILES        =
HFILES          = ${MSGHDRS}

EXPLIB_TARGETS  = export_libmsaa.a

ILIST           = libmsaa.a
IDIR            = /usr/ccs/lib/libp/

CFLAGS		= -qnofold -qdebug=nglobshd -D_SVID=1 -p

INCFLAGS= -I.. -I../${TARGET_MACHINE} \
-I${MAKETOP}/bos/usr/ccs/lib/libm -I${MAKETOP}/bos/usr/ccs/lib/libm/${TARGET_MACHINE}

OFILES		= \
	acos.o acosh.o asin.o asinh.o atan.o atan2.o \
	atanh.o cabs.o cbrt.o class.o copysign.o cos.o \
	cosh.o drem.o erf.o exp.o exp__E.o finite.o \
	floor.o isnan.o lgamma.o ilogb.o j0.o j1.o jn.o \
	log.o log10.o log1p.o logb.o log__L.o matherr.o \
	nextaft.o pow.o scalb.o sin.o sinh.o sqrt.o tan.o \
	tanh.o tantab.o unord.o acosF.o asinF.o atan2F.o \
	atanF.o cosF.o coshF.o erfF.o expF.o floorF.o \
	lgammaF.o log10F.o logF.o powF.o sinF.o sinhF.o \
	sqrtF.o tanF.o tanhF.o m_tables.o expm1.o \
	rsqrt.o rsqrtF.o	\
	${${TARGET_MACHINE}_OFILES}

.include <../${TARGET_MACHINE}/machdep.mk>

.include <${RULES_MK}>

#
# adding _CCFLAGS_ rule so that CENV can be set.
#

_CCFLAGS_=\
	${_CC_CFLAGS_}\
	${_CC_OL_}\
	${${.TARGET}_CENV:U${CENV}}\
	${${.TARGET}_CFLAGS:U${CFLAGS}} ${TARGET_FLAGS}\
	${${.TARGET}_CARGS:U${CARGS}}\
	${_CC_NOSTDINC_} ${_GENINC_} ${_CC_INCDIRS_}

#
# sqrt.o will be built using sqrt.s if sqrt.c is not used explicitly
# to build it.  The reason is that the VPATH is not order specific
# and thus if you have two files with the same name but different 
# extensions you are not guaranted a given result.
#
sqrt.o	: sqrt.c
	  ${RM} -f ${.TARGET}
	  ${CC} -c ${_CCFLAGS_} ${sqrt.c:P}

#
# ditto
#

sqrtF.o	: sqrtF.c 
	  ${RM} -f ${.TARGET}
	  ${CC} -c ${_CCFLAGS_} ${sqrtF.c:P}

