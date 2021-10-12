# @(#)33	1.9  src/bos/diag/da/fpa/Makefile, dafpa, bos411, 9428A410j 3/23/94 06:19:28
#
#   COMPONENT_NAME: DAFPA
#
#   FUNCTIONS: 
#
#   ORIGINS: 27, 83
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#   LEVEL 1, 5 Years Bull Confidential Information
#


PROGRAMS	= dfpp

MSGHDRS		= fpp_msg.h
CATFILES 	= fpp.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc

ILIST		= ${PROGRAMS} fpp.cat
IDIR		= /usr/lpp/diagnostics/da/
fpp.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dfpp_OFILES	= dfpp.o chkreg.o startup.o decfull.o expfull.o lzafull.o \
		  cmpp.o div.o fbd.o mlfs.o mlfsx.o mlfdx.o mlfsu.o \
		  mlfsux.o mlfdu.o mlfdux.o mstfs.o mstfsx.o mstfdx.o mstfsu.o \
		  mstfsux.o mstfdu.o mstfdux.o all.o chip.o fa.o fap.o fdd.o \
		  fdpp.o fm.o fm2.o fma.o fmap.o fmp.o frsp.o frspp.o \
		  mstfsu.o mtfsf.o fpfail2.o fpfail8.o sqrt.o sqrtp.o sys.o

.include <${RULES_MK}>
