# @(#)45	1.1  src/bldenv/libIN/Makefile, ade_build, bos41J, 9513A_all 3/22/95 11:20:23
#
#   COMPONENT_NAME: ADE_BUILD
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/usr/ccs/lib/libIN:${MAKETOP}/bos/usr/include

LIBRARIES	= libIN.a
MSGHDRS		= libIN_msg.h
CATFILES	= libIN.cat
LOCAL_HFILES	= include/gpoff.h
HFILES		= ${MSGHDRS} ${LOCAL_HFILES}

EXPLIB_TARGETS	= export_libIN.a

ILIST		= libIN.a
IDIR		= /usr/lib/

libIN_msg.h_IDIR	= /usr/include/
INCLUDES	= libIN_msg.h

CFLAGS		= -D_AIXFS
INCFLAGS	+= -Iinclude

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

${LOCAL_HFILES} : $${.TARGET:T}
	${RM} ${_RMFLAGS_} ${.TARGET} ${.TARGET:T}
	[ -d ${.TARGET:H} ] || ${MKDIR} ${.TARGET:H}
	${CP} ${${.TARGET:T}:P} ${.TARGET}
