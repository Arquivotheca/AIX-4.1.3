# @(#)25        1.14  src/bos/usr/ccs/lib/libc/POWER/machdep.mk, libc, bos411, 9428A410j 4/26/94 13:59:20
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 10,27,83
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

INCFLAGS += -I${MAKETOP}bos/kernel/ml/${TARGET_MACHINE}

POWER_SHARED_OFILES = \
	_q_cvtl.o _q_mp10.o _qatof.o _qcvt.o _qdbcmp.o	\
	_qfloor.o _qint.o _qitrunc.o _qnint.o _xlqadd.o	\
	_xlqdiv.o _xlqsub.o atof_const.o atof_tab.o clc.o \
	cvtloop.o dsto2fp.o fp_raise.o fp_sh_code.o	\
	fp_trap.o fsavres.o imul_dbl.o longjmp.o sync_cache.o macros.o	\
	mf2x2.o shr_itrunc.o udiv.o umul_dbl.o w31stuff.o	\
	wcdsto2fp.o wcstod.o fp_trapstate.o fp_flush_imprecise.o \
	f128toi64rz.o f128tou64rz.o f64toi64rz.o f64tou64rz.o	\
	divi64.o divu64.o compi64.o compu64.o	\
	multi64.o _xlqsub.o \
	strtol.o strtoul.o strtoll.o strtoull.o \
	wcstol.o wcstoul.o wcstoll.o wcstoull.o \
	lldiv.o strtold.o frexpl.o ldexpl.o modfl.o atomic_op.o \
	read_real_time.o time_base_to_time.o

POWER_NOSHARED_OFILES= 	\
	_qint.o _qitrunc.o _qnint.o _setflm.o _xlqadd.o	\
        _xlqdiv.o _xlqmul.o _xlqsub.o clc.o fsavres.o	\
        longjmp.o maxi64.o mini64.o multi64.o compi64.o compu64.o \
	llabs.o _quitrunc.o

POWER_SYS_OFILES	= \
	udiv.o multi64.o compi64.o compu64.o divi64.o divu64.o
