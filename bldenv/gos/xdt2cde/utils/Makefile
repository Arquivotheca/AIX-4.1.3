# @(#)64	1.2  src/bldenv/gos/xdt2cde/utils/Makefile, xdt2cde, gos411, 9428A410i 3/11/94 13:45:41
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

VPATH           = ${MAKETOP}gos/xdt2cde/utils/byacc

PROGRAMS	= byacc flex

CFLAGS		= -DMOTIF_1_1_2 -DUSE_WAITPID -DIXISYS_rsai0302 \
		  -DIXISYS_rsai=0x0302  -DIXISYS_CODE=\"rsai0302\" \
		  -DOSAL_CHECK_HEAP -DOSAL_CHECK_STRLEN 

ILIST		= ${PROGRAMS}
IDIR 		= /xdt2cde/bin/

INCFLAGS	= -I${MAKETOP}gos/xdt2cde/ixihdrs \
		  -I${MAKETOP}gos/xdt2cde/osal \
		  -I${MAKETOP}gos/xdt2cde/ixilib \
	          -I${MAKETOP}gos/xdt2cde/xqplib \
		  -I${MAKETOP}gos/xdt2cde/dragdroplib 

LIBS		= ../osal/ixi_osal.a 

byacc_OFILES	= by_closure.o  by_error.o   by_lalr.o    by_lr0.o    \
	          by_main.o     by_mkpar.o   by_output.o  by_reader.o \
	          by_skeleton.o by_symtab.o  by_verbose.o by_warshall.o 

flex_OFILES	= fl_ccl.o     fl_dfa.o     fl_ecs.o     fl_gen.o   \
		  fl_main.o    fl_misc.o    fl_nfa.o     fl_parse.o \
		  fl_scan.o    fl_sym.o     fl_tblcmp.o  fl_yylex.o

.include <${RULES_MK}>

