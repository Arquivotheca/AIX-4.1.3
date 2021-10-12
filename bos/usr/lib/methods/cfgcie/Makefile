#  @(#)67   1.4  src/bos/usr/lib/methods/cfgcie/Makefile, sysxcie, bos411, 9428A410j 4/1/94 15:27:43
#
#
#  COMPONENT_NAME: (SYSXCIE) COMIO Emulator
#
#  DESCRIPTION:
#     Configuration Method Makefile
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

CIE	     = ${MAKETOP}bos/kernext/cie/
COMMON	     = ${MAKETOP}bos/usr/lib/methods/common/
VPATH	     = ../common/

PROGRAMS	= cfgcie ucfgcie
CATFILES        = ciedd.cat
INCFLAGS	= -I${CIE} -I${COMMON}
LIBS		= -lcfg -lodm
CFLAGS	       += -qcpluscmt -qsource -qlist -qsrcmsg
OPT_LEVEL	= -O

ILIST		= cfgcie ucfgcie ciedd.cat
IDIR		= /usr/lib/methods/

cfgcie_OFILES	= cfgcie.o cfgutil.o cfgtoolsx.o
ucfgcie_OFILES	= ucfgcie.o cfgutil.o cfgtoolsx.o

.include <${RULES_MK}>
