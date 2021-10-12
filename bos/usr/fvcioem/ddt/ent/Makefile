#  @(#)96   1.1  src/bos/usr/fvcioem/ddt/ent/Makefile, fvcioem, bos411, 9428A410j 4/26/94 13:54:16
#
#  COMPONENT_NAME: (sysxcie_tc) COMIO Emulator Test Tools
#
#  DESCRIPTION:
#     Ethernet DDT Test Tool Makefile
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
PROGRAMS	= entddt
OFILES		= ddt_com.o ddt_ent.o ddtlog.o
CFLAGS	       += -O -qcpluscmt
INCFLAGS	= -I../common -I../etkid
LIBFLAGS       += -L${MAKETOP}/../export/power/usr/ccs/lib
LIBS		= -lIN -lPW

.include <${RULES_MK}>
