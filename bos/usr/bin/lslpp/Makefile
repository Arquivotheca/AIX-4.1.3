# @(#)76        1.4 src/bos/usr/bin/lslpp/Makefile, cmdswvpd, bos411, 9432A411a 8/5/94 09:38:42
#
# COMPONENT_NAME: CMDSWVPD
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1988,1993
# All Rights Reserved
# Licensed Materials - Property of IBM
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/usr/bin/lppchk

PROGRAMS	= lslpp
MSGHDRS		= swvpd_msg.h
CATFILES	= swvpd.cat
HFILES		= ${MSGHDRS}

ILIST		= lslpp
IDIR		= /usr/bin/

INCFLAGS	=-I../../sbin/install/include -I../../sbin/install/ckprereq -I../../sbin/install/messages -I../../sbin/install/sysck

OFILES		= lslpp.o process_L_flag.o swvpd_str.o lslpp_utils.o

LIBS		= -lswvpd -lodm -lckprereq -linulib 

.include <${RULES_MK}>
