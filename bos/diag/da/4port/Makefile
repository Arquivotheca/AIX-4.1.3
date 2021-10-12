# @(#)24	1.2  src/bos/diag/da/4port/Makefile, da4port, bos411, 9428A410j 12/5/93 20:17:47
#
#   COMPONENT_NAME: DA4PORT
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#



VPATH		= ${MAKETOP}/bos/diag/tu/4port

PROGRAMS	= dmultp

MSGHDRS		= dmultp_msg.h
CATFILES 	= dmultp.cat
HFILES		= ${MSGHDRS}
CFLAGS		= -D DIAG

LIBS		= -lasl -ldiag -lodm -lcfg -lc

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/4port

ILIST		= ${PROGRAMS} dmultp.cat
IDIR		= /usr/lpp/diagnostics/da/
dmultp.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

TUS		= exectu.o tu01.o tu02.o tu03.o tu04.o tu05.o tu06.o tu07.o \
	        	tu08.o tu09.o tu10.o tu11.o tu12.o tu13.o tu13a.o \
	        	tu13b.o tu13c.o tu13d.o tu14.o tu15.o tu15a.o tu15b.o \
	        	tu15c.o tu15d.o tu16.o tu16a.o tu16b.o tu16c.o tu16d.o \
                	tu17.o read_timers.o getstatus.o ld_adp_sw.o ioread.o \
			start.o halt.o iowrite.o

DABASE		= mpqpmenus.o dompqptu.o dompqptest.o mpqp_main.o \
			asnmpqpf.o asl_rtn.o get_mode.o

dmultp_OFILES	= ${DABASE} ${TUS}

.include <${RULES_MK}>
