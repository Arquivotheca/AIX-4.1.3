# @(#)59  1.5  src/bos/diag/da/artic/Makefile, daartic, bos41J, 9518A_all 5/1/95 14:29:18
#
#   COMPONENT_NAME: DAARTIC
#
#   FUNCTIONS: None 
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/diag/tu/artic

PROGRAMS	= dartic

MSGHDRS		= dartic_msg.h dmultp_msg.h dpm_msg.h dx25_msg.h dxm2p_msg.h \
                  dc1x_msg.h 

CATFILES 	= dartic.cat   dmultp.cat   dpm.cat   dx25.cat   dxm2p.cat \
                  dc1x.cat

HFILES		= ${MSGHDRS}
INCFLAGS	= -I${MAKETOP}/bos/diag/tu/artic

LIBS		= -lasl -ldiag -lodm -lcfg -lc -lartictu

ILIST		= ${PROGRAMS} ${CATFILES}
IDIR		= /usr/lpp/diagnostics/da/
dartic.cat_IDIR = /usr/lpp/diagnostics/catalog/default/
dmultp.cat_IDIR = /usr/lpp/diagnostics/catalog/default/
dpm.cat_IDIR 	= /usr/lpp/diagnostics/catalog/default/
dx25.cat_IDIR 	= /usr/lpp/diagnostics/catalog/default/
dxm2p.cat_IDIR 	= /usr/lpp/diagnostics/catalog/default/
dc1x.cat_IDIR 	= /usr/lpp/diagnostics/catalog/default/

dartic_OFILES	= dartic.o dmultp.o dx25.o dpm.o dmp2.o display.o dt1e1j1.o

.include <${RULES_MK}>
