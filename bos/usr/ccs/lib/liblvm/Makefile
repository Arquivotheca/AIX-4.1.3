# @(#)24        1.3  src/bos/usr/ccs/lib/liblvm/Makefile, liblvm, bos411, 9428A410j 2/22/93 16:14:07
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989,
# 			1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#



SHARED_LIBRARIES	= liblvm.a
MSGHDRS         =
CATFILES        =
HFILES          = ${MSGHDRS}

EXPLIB_TARGETS  = export_liblvm.a

ILIST           = liblvm.a
IDIR            = /usr/lib/

LIBS		= libc.a libcfg.a librts.a libodm.a
EXPORTS		= -bE:liblvm.exp

SHARED_OFILES	= \
	bbdirutl.o bblstutl.o comutl.o changelv.o changepv.o \
	chkquorum.o configutl.o createlv.o createvg.o crtinsutl.o \
	deletelv.o deletepv.o extendlv.o installpv.o lvmrecutl.o \
	migratepp.o mkuuid.o querylv.o querypv.o queryutl.o queryvg.o \
	queryvgs.o rdex_com.o reducelv.o resynclp.o resynclv.o resyncpv.o \
	revaryon.o setupvg.o synclp.o utilities.o varyoffvg.o varyonvg.o \
	verify.o vonutl.o wrtutl.o

.include <${RULES_MK}>
