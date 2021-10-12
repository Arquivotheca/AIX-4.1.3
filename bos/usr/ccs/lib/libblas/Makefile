# @(#)26        1.4  src/bos/usr/ccs/lib/libblas/Makefile, libblas, bos411, 9428A410j 4/6/93 09:51:59
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



LIBRARIES	= libblas.a
MSGHDRS         = bl_err_msg.h
CATFILES 	= bl_err.cat
HFILES          = ${MSGHDRS}

EXPLIB_TARGETS  = export_libblas.a

ILIST           = libblas.a
IDIR            = /usr/lib/

OFILES		= \
	bl_err.o caxpy.o ccopy.o cdotc.o cdotu.o cgbmv.o \
	cgemm.o	cgemv.o	cgerc.o	cgeru.o	chbmv.o	chemm.o	\
	chemv.o	cher.o cher2.o cher2k.o	cherk.o	chpmv.o	\
	chpr.o chpr2.o	crotg.o	cscal.o	csrot.o	csscal.o \
	cswap.o	csymm.o	csyr2k.o csyrk.o ctbmv.o ctbsv.o \
	ctpmv.o	ctpsv.o	ctrmm.o	ctrmv.o	ctrsm.o	ctrsv.o	\
	dab.o	dabt.o	dasum.o	datb.o	datbt.o	daxpy.o	\
	daxpyU.o dbrtl.o dbrtu.o dcabs1.o dcopy.o ddot.o \
	dgbmv.o	dgemm.o	dgemmU.o dgemv.o dgemvU.o dger1.o \
	dger1U.o dgerb.o dgetm1.o dgetmo.o dmbl.o dmbu.o \
	dmlb.o	dmm.o	dmub.o	dmv.o dmv4.o dmv5.o dnrm2.o \
	drk1.o drot.o drotg.o drotm.o drotmg.o dsbmv.o dscal.o \
	dsidl.o	dsidr.o	dslvl.o	dslvu.o	dsnt.o dspmv.o dspr.o	\
	dspr2.o	dswap.o	dsymm.o	dsymv.o	dsyr.o dsyr2.o dsyr2k.o	\
	dsyrk.o	dtabt.o	dtabx.o	dtata.o	dtatb.o	dtbmv.o	dtbsv.o	\
	dtfr.o dtfrm.o dtmm.o dtpmv.o dtpsv.o dtrca.o dtrmb.o	\
	dtrmm.o	dtrmv.o	dtrsa.o	dtrsb.o	dtrsm.o	dtrsv.o	dxfrf.o	\
	dxfrt.o	dyax.o dzasum.o dznrm2.o icamax.o idamax.o idamaxU.o \
	irline.o isamax.o isamaxU.o izamax.o lsame.o sab.o sabt.o \
	sasum.o	satb.o satbt.o saxpy.o saxpyU.o sbrtl.o sbrtu.o	\
	scasum.o scnrm2.o scopy.o sdot.o sdsdot.o sgbmv.o sgemm.o \
	sgemmU.o sgemv.o sgemvU.o sger1.o sger1U.o sgerb.o sgetm1.o	\
	sgetmo.o smbl.o	smbu.o smlb.o smm.o smub.o smv.o smv4.o	smv5.o	\
	snrm2.o	srk1.o srot.o srotg.o srotm.o srotmg.o ssbmv.o sscal.o	\
	ssidl.o	ssidr.o	sslvl.o	sslvu.o	ssnt.o sspmv.o sspr.o sspr2.o	\
	sswap.o	ssymm.o	ssymv.o	ssyr.o ssyr2.o ssyr2k.o ssyrk.o	stabt.o	\
	stabx.o	stata.o	statb.o	stbmv.o	stbsv.o	stfr.o stfrm.o stmm.o	\
	stpmv.o	stpsv.o	strca.o	strmb.o	strmm.o	strmv.o	strsa.o	strsb.o	\
	strsm.o	strsv.o	sxfrf.o	sxfrt.o	syax.o xerbla.o zaxpy.o zcopy.o \
	zdotc.o	zdotu.o	zdrot.o	zdscal.o zgbmv.o zgemm.o zgemv.o zgerc.o \
	zgeru.o zhbmv.o zhemm.o zhemv.o zher.o zher2.o zher2k.o zherk.o	\
	zhpmv.o zhpr.o zhpr2.o zrotg.o zscal.o zswap.o zsymm.o zsyr2k.o	\
	zsyrk.o ztbmv.o ztbsv.o ztpmv.o ztpsv.o ztrmm.o ztrmv.o ztrsm.o	\
	ztrsv.o


.include <${RULES_MK}>
