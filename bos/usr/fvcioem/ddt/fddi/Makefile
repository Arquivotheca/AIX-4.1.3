#  @(#)05   1.1  src/bos/usr/fvcioem/ddt/fddi/Makefile, fvcioem, bos411, 9428A410j 4/26/94 13:54:39
#
#  COMPONENT_NAME: (sysxcie_tc) COMIO Emulator Test Tools
#
#  DESCRIPTION:
#     FDDI DDT Test Tool Makefile
#
#  ORIGINS: 27
#
#  (C) COPYRIGHT International Business Machines Corp. 1988, 1994
#  All Rights Reserved
#  Licensed Materials - Property of IBM
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ../common
PROGRAMS	= fddiddt
OFILES		= ddt_com.o ddt_fddi.o ddtlog.o
CFLAGS	       += -O -qcpluscmt -qsource -qlist -qsrcmsg
INCFLAGS	= -I../common -I../fddikid -I${MAKETOP}bos/kernext/fddi
LIBFLAGS       += -L${MAKETOP}/../export/power/usr/ccs/lib
LIBS		= -lIN -lPW -lodm -lcfg

.include <${RULES_MK}>
