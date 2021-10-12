# @(#)00	1.1  src/bos/diag/da/gga/Makefile, dagga, bos41J, 9515A_all 4/6/95 09:46:52
#
#   COMPONENT_NAME: DAGGA
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


VPATH		= ${MAKETOP}/bos/diag/tu/gga:${MAKETOP}/bos/kernext/exp
PROGRAMS	= dgga

MSGHDRS		= dgga_msg.h
CATFILES 	= dgga.cat
HFILES		= ${MSGHDRS}

CFLAGS		+= ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

LIBS		= -lasl -ldiag -lodm -lcfg -lc -ltu_gga

INCFLAGS        = -I${MAKETOP}/bos/usr/include \
		-I${MAKETOP}/bos/diag/tu/gga

ILIST		= ${PROGRAMS} dgga.cat
IDIR		= /usr/lpp/diagnostics/da/
dgga.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dgga_IMPORTS	= -bI:rcm.exp

dgga_OFILES	= dgga.o

.include <${RULES_MK}>
