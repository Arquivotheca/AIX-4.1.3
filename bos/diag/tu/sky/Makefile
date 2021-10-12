# @(#)54        1.3  src/bos/diag/tu/sky/Makefile, tu_sky, bos411, 9428A410j 2/15/94 16:04:11
#
#   COMPONENT_NAME : (tu_sky) Color Graphics Display Adapter Test Units
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1989,1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


LIBRARIES	= libskCtu.a

INCFLAGS        = -I${MAKETOP}/bos/kernext/disp/sky \
                  -I${MAKETOP}/bos/kernext/disp/sky/inc

EXPLIB_TARGETS  = export_libskCtu.a
EXPDIR		= /usr/lib/


OFILES		 = \
        Sig.o   \
        a000.o  \
        a0f0.o  \
        b000.o  \
        c0.o    \
        c030.o  \
        c050.o  \
        c061.o  \
        c062.o  \
        c064.o  \
        c065.o  \
        c0b2.o  \
        c0b3.o  \
        c0f1.o  \
        e000.o  \
        e010.o  \
        e020.o  \
        e030.o  \
        exectu.o \
        p010.o  \
        p110.o  \
        p210.o  \
        p215.o  \
        p220.o  \
        p510.o  \
        p511.o  \
        p610.o  \
        p620.o  \
        p6a0.o  \
        pa10.o  \
        pa13.o  \
        pa80.o  \
        pe10.o  \
        rcmsg.o \
        runtu.o \
        rw.o    \
        setpal.o \
        skdflts.o \
        skytu.o

LIBS		= -lc -lm

.include <${RULES_MK}>
