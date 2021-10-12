# @(#)65	1.2  src/bldenv/gos/xdt2cde/osal/Makefile, xdt2cde, gos411, 9428A410i 3/11/94 13:42:20
#
#   COMPONENT_NAME: XDT2CDE
#
#   FUNCTIONS: none
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

VPATH		= ${MAKETOP}gos/xdt2cde/osal

LIBRARIES	= ixi_osal.a

CFLAGS          = -DMOTIF_1_1_2 -DUSE_WAITPID -DIXISYS_rsai0302  \
		  -DIXISYS_rsai=0x0302  -DIXISYS_CODE=\"rsai0302\" \
		  -DOSAL_CHECK_HEAP -DOSAL_CHECK_STRLEN \
		  -DOSAL_NLS_DEFAULT_PATH=\"/usr/lib/X11/XDesktop3/messages/%L\"

INCFLAGS        = -I${MAKETOP}gos/xdt2cde/ixihdrs \
		  -I${MAKETOP}gos/xdt2cde/ixilib \
		  -I${MAKETOP}gos/xdt2cde/xqplib \
		  -I${MAKETOP}gos/xdt2cde/dragdroplib \
		  -I/usr/include

OFILES          = getcwd_e.o   heap.o       heapcheck.o  mkdir_e.o  \
	          nls_e.o      osalextras.o rename_e.o   scandir_e.o \
	          setlocal_e.o signals.o     strerror_e.o strlen.o \
		  symlinks_e.o sysconf_e.o  timesp.o      uname_e.o \
	          utime_e.o    slowSRstat_e.o ctype_e.o

.include <${RULES_MK}>

