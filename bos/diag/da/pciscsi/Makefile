# @(#)50        1.1  src/bos/diag/da/pciscsi/Makefile, dapciscsi, bos41J, 9514A_all 4/4/95 21:10:50
#
#   COMPONENT_NAME: DAPCISCSI
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#
#

VPATH           = ${MAKETOP}/bos/kernext/exp

PROGRAMS	= dpciscsi

MSGHDRS         = dpciscsi_msg.h

CATFILES        = dpciscsi.cat

HFILES          = ${MAKETOP}/bos/diag/tu/bloom/bloomtu.h ${MSGHDRS} 

INCFLAGS        = -I${MAKETOP}/bos/diag/tu/bloom

LIBS            = -lasl -ldiag -lodm -lcfg -lc -lbloomtu

ILIST           = ${PROGRAMS} ${CATFILES}

IDIR            = /usr/lpp/diagnostics/da/

dpciscsi.cat_IDIR        = /usr/lpp/diagnostics/catalog/default/

dpciscsi_OFILES  = pciscsi.o

dpciscsi_IMPORTS	= -bI:diagex.exp 

.include <${RULES_MK}>

