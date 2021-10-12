# @(#)34	1.6  src/bos/usr/lpp/kls/dictutil/Makefile, cmdkr, bos411, 9428A410j 2/21/94 08:41:46
#
# COMPONENT_NAME:  CMDKR
#
# FUNCTIONS: Korean User Dictionary Utility 
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

CFLAGS += -DTERMH_CH -D_NO_PROTO ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURSESLIBFLAG}

NOBINDGROUPS = dictmain_obj

PROGRAMS = dictmain
OTHERS   = hdict.cat
CATFILES = hdict.cat
HFILES   = hhccomm.h hucode.h hucommon.h huconst.h huke.h humacros.h hut.h

OFILES = \
	hudichnd.o hubket.o hudcmrua.o hudcmrud.o hudcread.o \
	hudcutil.o hudcwrit.o hudeadpr.o hudedtsc.o hudeixsc.o huderepl.o \
	hudicadh.o hudicadp.o hudiccom.o hudiccs.o hudicdlb.o hudicdlc.o \
	hudicdlp.o hudickc.o hudicrcv.o hudicscb.o hudicueh.o hudicupc.o \
	hudicuph.o hudicupm.o hudisply.o humsg.o hufnc.o hufout.o \
	hugetc.o hugetcmp.o hugeti.o hugetkey.o huipfld.o humvch.o \
	hushow.o husize.o hutable.o hutmwrt.o

ILIST 	       = $(PROGRAMS) hdict.kor hdict hdict.cat
IDIR	       = /usr/lpp/kls/bin/
hdict_IDIR     = /usr/bin/
hdict.cat_IDIR = /usr/lpp/kls/etc/

LIBS	= -lcurses

.include <${RULES_MK}>
