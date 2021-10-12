# @(#)21        1.1  src/bos/diag/da/pcient/Makefile, dapcient, bos41J, 9514A_all 4/4/95 16:13:47
#
#   COMPONENT_NAME: dapcient
#
#   FUNCTIONS: None
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


VPATH		= ${MAKETOP}/bos/kernext/exp:${MAKETOP}/rspc/kernext/pci/kent
PROGRAMS	= dpcient

OTHERS          = kent_errids.h

MSGHDRS		= dpcient_msg.h
CATFILES 	= dpcient.cat
HFILES		= ${MSGHDRS}

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/ktat

LIBS		= -ldiag -lasl -lodm -lcfg -lc -lktattu

ILIST		= ${PROGRAMS} dpcient.cat
IDIR		= /usr/lpp/diagnostics/da/
dpcient.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dpcient_OFILES          = dpcient.o

dpcient_IMPORTS = -bI:diagex.exp

kent_errids.h: devices.pci.22100020.rte.err.S
		${CP} ${devices.pci.22100020.rte.err.S:P} .
		${ERRTMPLTBLD} -d devices.pci.22100020.rte.err.S -o kent_errids.h
		${RM} -f devices.pci.22100020.rte.err.S


.include <${RULES_MK}>
