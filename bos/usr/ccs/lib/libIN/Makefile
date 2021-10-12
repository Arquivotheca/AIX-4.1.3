# @(#)88        1.3  src/bos/usr/ccs/lib/libIN/Makefile, libIN, bos411, 9428A410j 11/24/93 14:26:55
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 10,27
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

LIBRARIES	= libIN.a
MSGHDRS		= libIN_msg.h
CATFILES	= libIN.cat
HFILES		= ${MSGHDRS}

EXPLIB_TARGETS	= export_libIN.a

ILIST		= libIN.a
IDIR		= /usr/lib/

libIN_msg.h_IDIR	= /usr/include/
INCLUDES	= libIN_msg.h

CFLAGS		 = -D_AIXFS

OFILES = \
	AOcanon.o AOdesc.o ARfuncs.o CAcpy.o CAfill.o \
	CAopen.o CAtr.o CScat.o CScmp.o CScmpp.o CScpy.o \
	CScpym.o CScpyn.o CScpyu.o CScpyum.o CScurdir.o \
	CSdevname.o CSdname.o CSgetl.o CSgets.o CSlen.o \
	CSloca.o CSlocc.o CSlocs.o CSloct.o CSnil.o \
	CSprintf.o CSskpa.o CSskpc.o CSskps.o CSskpt.o \
	CSsname.o CStol.o DLfree.o DLlock.o DRget.o \
	ERcmderr.o ERsysmsg.o FSdskname.o LGdev.o LGdir.o \
	LGname.o LS1append.o LS1concat.o LS1count.o LS1insert.o \
	LS1killall.o LS1prepend.o LS1rmvfirs.o LS1rmvnext.o \
	LS2append.o LS2concat.o LS2count.o LS2insaft.o \
	LS2insbef.o LS2killall.o LS2prepend.o LS2remove.o \
	LS2rmvfirs.o LS2rmvlast.o MSfcns.o PFageless.o PFclose.o \
	PFgdata.o PFgidgrp.o PFgrpgid.o PFid.o PFname.o \
	PFnext.o PFopen.o PFudata.o PFuidusr.o PFusrdir.o \
	PFusrgid.o PFusruid.o SGcodes.o SGnames.o XX.o \
	XXauxent.o XXheader.o XXlineno.o XXreloc.o XXsyment.o \
	eprintf.o sysv.o

.include <${RULES_MK}>
