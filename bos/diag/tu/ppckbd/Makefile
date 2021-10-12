# @(#)71  1.1  src/htx/usr/lpp/htx/lib/ppckbd/Makefile, tu_ppckbd, htx410 2/1/95 13:59:55
#
#   COMPONENT_NAME: tu_ppckbd
#
#   FUNCTIONS: none
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
###########################################################################

LIBRARIES	= libppckbd.a

INCLUDES  = 

EXPLIB_TARGETS  = export_libppckbd.a
EXPDIR		= /usr/lib/

VPATH           = ${MAKETOP}/bos/diag/tu/ppckbd

CFLAGS          = -DDIAGNOSTICS ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}

INCFLAGS        = -I${MAKETOP}/bos/diag/tu/ppckbd
MSGHDRS         = disakbd_msg.h
CATFILES        = disakbd.cat
HFILES          = ${MSGHDRS}

LIBS            = ${_AIX3CURLIB} ${_AIX3CURSESLIB}

OFILES		 = \
        exectu.o \
	mktu_rc.o \
	kbd_fns.o \
	kbd_open.o \
	sem.o	\
        tu10.o \
        tu20.o \
        tu30.o \
        tu40.o \
	kbd_msg.o

ILIST           = disakbd.cat
IDIR            = /usr/lpp/diagnostics/catalog/default/

.include <${RULES_MK}>
