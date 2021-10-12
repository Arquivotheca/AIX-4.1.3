# @(#)83        1.11.1.10  src/bos/usr/lpp/jls/dictutil/Makefile, cmdKJI, bos411, 9428A410j 3/4/94 14:04:35
#
# COMPONENT_NAME: cmdKJI
#
# FUNCTIONS: User Dictionary Utility
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1992, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#	Makefile for dictutil     
#

PROGRAMS	= kjdict

CFLAGS		+= -DTERMH_CH -D_NO_PROTO -DNULL=0 -DRS6000=1 -DEXTCUR=1 \
		-DCNVSTOC=1 -DPARMADD=1 -DMSGFILE=1 -DCNVEVT=1 \
		${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}

LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

ILIST		= ${PROGRAMS}
IDIR		= /usr/bin/

LIBS            = -lcur -lcurses -liconv

OFILES		= kjcnvets.o \
			kjcnvste.o \
			kjdicadd.o \
			kjdicpad.o \
			kjflchk.o  \
			kjinit.o   \
			kjinput.o  \
			kjprint.o  \
			kjprmchk.o \
			kjprmsg.o  \
			kuconvm.o  \
			kudcmrua.o \
			kudcmrud.o \
			kudcread.o \
			kudcwrit.o \
			kudeadpr.o \
			kudedtsc.o \
			kudeixsc.o \
			kuderepl.o \
			kudicadh.o \
			kudicadp.o \
			kudiccom.o \
			kudiccs3.o \
			kudicdlb.o \
			kudicdlc.o \
			kudicdlp.o \
			kudiceng.o \
			kudichnd.o \
			kudicmlc3.o \
			kudicrcv.o \
			kudicscb.o \
			kudicueh.o \
			kudicupc.o \
			kudicuph.o \
			kudicupm.o \
			kudicycr.o \
			kudicymc.o \
			kudisply.o \
			kudisply2.o \
			kufnc.o    \
			kufout.o   \
			kugetc.o   \
			kugetcmp.o \
			kugeti.o   \
			kugetkey.o \
			kugnbkf.o  \
			kuhkfc.o   \
			kuhtdc.o   \
			kuipfld.o  \
			kumdinfo.o \
			kumdsrch.o \
			kumsg.o    \
			kumvch.o   \
			kuopbkf.o  \
			kushow.o   \
			kusize.o   \
			kusysrd.o  \
			kutable.o  \
			kutmwrt.o  \
			kuusrrd.o  \
			main.o

.include <${RULES_MK}>
