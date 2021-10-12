# @(#)51	1.5  src/bos/diag/da/fd/Makefile, dafd, bos411, 9428A410j 2/21/94 08:39:10
#
#   COMPONENT_NAME: DAFD
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= dfd dfda

MSGHDRS		= dfd_msg.h
CATFILES 	= dfd.cat
HFILES		= ${MSGHDRS}

CFLAGS		+= ${_AIX3CURFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG}

LIBS		= -lasl -lcur -ldiag -lodm -lcfg -lc -lfdtu

ILIST		= ${PROGRAMS} dfd.cat
IDIR		= /usr/lpp/diagnostics/da/

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/fd

dfd.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dfd_OFILES	= do_fd_test.o do_fd_tu.o fd_main.o asl_rtn.o asn_fd_fru.o \
			get_mode.o dfd_menus.o

dfda_OFILES	= dfda_menus.o asl_rtn.o fda_main.o get_mode.o asn_fda_f.o

.include <${RULES_MK}>
