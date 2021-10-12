# @(#)72        1.6  src/bos/diag/da/x25/Makefile, dax25, bos411, 9428A410j 12/5/93 20:15:17
#
#   COMPONENT_NAME: DAX25
#
#   FUNCTIONS:
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1993.
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


VPATH           = ${MAKETOP}/bos/diag/tu/x25
PROGRAMS        = dx25

MSGHDRS         = dx25_msg.h
CATFILES        = dx25.cat
HFILES          = ${MSGHDRS}
INCFLAGS        = -I${MAKETOP}/bos/diag/tu/x25

LIBS            = -lasl -ldiag -lodm -lcfg -lc


tu_OFILES       = card_rr.o rw_io.o rw_mem.o st_task.o tu019.o

ILIST           = ${PROGRAMS} dx25.cat f0efd.00.01
IDIR            = /usr/lpp/diagnostics/da/
dx25.cat_IDIR = /usr/lpp/diagnostics/catalog/default/
f0efd.00.01_IDIR = /usr/lib/microcode/

TUS             = card_rr.o check_wrap.o chk_gate.o detrace.o exectu.o \
	          out_br.o rcp_stat.o reset_card.o reset_task.o rw_io.o \
	          rw_mem.o st_diag.o st_task.o tu001.o tu003.o tu004.o \
	          tu005.o tu006.o tu007.o tu008.o tu010.o tu011.o tu012.o \
	          tu013.o tu014.o tu015.o tu016.o tu017.o tu018.o tu019.o

dx25_OFILES     = ${TUS} dx25.o

.include <${RULES_MK}>
