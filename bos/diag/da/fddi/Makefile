# @(#)17	1.8  src/bos/diag/da/fddi/Makefile, dafddi, bos411, 9428A410j 2/21/94 15:27:37
#
#   COMPONENT_NAME: DAFDDI
#
#   FUNCTIONS: None 
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1991,1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#



VPATH		= ${MAKETOP}/bos/diag/tu/fddi:${MAKETOP}/bos/kernext/fddi
PROGRAMS	= dfddi

OTHERS		= fddi_errids.h

MSGHDRS		= dfddi_msg.h
CATFILES 	= dfddi.cat
HFILES		= ${MSGHDRS}
INCFLAGS	= -I${MAKETOP}/bos/diag/tu/fddi -I${MAKETOP}/bos/kernext/fddidiag

LIBS		= -lasl -ldiag -lodm -lcfg -lc

ILIST		= ${PROGRAMS} dfddi.cat
IDIR		= /usr/lpp/diagnostics/da/
dfddi.cat_IDIR = /usr/lpp/diagnostics/catalog/default/

TUS	= detrace.o exectu.o st_fddi.o sr_regs.o sr_pos.o rw_bus_prg.o \
	rw_pos.o rw_sif.o get_port.o rw_allmem.o rw_datastore.o \
	rw_diag_tst.o chk_diag_tst.o mktu_rc.o tu001.o tu002.o  \
	tu003.o tu004.o tu005.o tu006.o tu007.o tu008.o tu009.o \
	tu010.o tu011.o tu012.o tu013.o tu014.o tu015.o tu016.o \
	tu017.o tu018.o tu019.o tu020.o tu021.o tu022.o tu023.o \
	tu024.o tu025.o tu026.o tu027.o tu028.o tu029.o tu030.o \
	tu031.o tu032.o tu033.o tu034.o tu035.o tu036.o tu037.o 

dfddi_OFILES	= ${TUS} dfddi.o 

.include <${RULES_MK}>

fddi_errids.h: devices.mca.8ef4.rte.err.S
		${CP} ${devices.mca.8ef4.rte.err.S:P} .
		${ERRTMPLTBLD} -d devices.mca.8ef4.rte.err.S -o fddi_errids.h
		${RM} -f devices.mca.8ef4.rte.err.S


