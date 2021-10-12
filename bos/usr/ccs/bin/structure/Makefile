# @(#)48        1.2  src/bos/usr/ccs/bin/structure/Makefile, cmdprog, bos411, 9428A410j 1/19/93 12:58:59
# COMPONENT_NAME: (CMDPROG) Programming Utilities
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1991
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS		= structure
SCRIPTS			= struct
MSGHDRS                 = struct_msg.h
CATFILES                = struct.cat

ILIST			= structure  struct
IDIR			= /usr/lib/struct/
struct_IDIR		= /usr/ucb/

CFLAGS			= -DBSD_INCLUDES
HFILES			= ${MSGHDRS} 

OFILES			= main.o \
			  0.alloc.o   0.args.o    0.def.o    0.extr.o \
			  0.graph.o   0.list.o	  0.parts.o  0.string.o \
			  1.finish.o  1.form.o    1.fort.o   1.hash.o  \
			  1.init.o    1.line.o    1.main.o   1.node.o  \
			  1.recog.o   1.tables.o  \
			  2.dfs.o     2.dom.o	  2.head.o   2.inarc.o  \
			  2.main.o    2.tree.o    \
			  3.branch.o  3.flow.o    3.loop.o   3.main.o   \
  			  3.reach.o   3.then.o    \
			  4.brace.o   4.form.o    4.main.o   4.out.o

LIBS			= -ll -lbsd

.include <${RULES_MK}>
