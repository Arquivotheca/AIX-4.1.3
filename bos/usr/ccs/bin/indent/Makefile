# @(#)50        1.1  src/bos/usr/ccs/bin/indent/Makefile, cmdprog, bos411, 9428A410j 8/5/92 16:27:49
# COMPONENT_NAME:
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

PROGRAMS		= indent
MSGHDRS                 = indent_msg.h 
CATFILES                = indent.cat

ILIST			= indent
IDIR			= /usr/ccs/bin/

CFLAGS			= 
HFILES			= ${MSGHDRS}  ind_globs.h  ind_codes.h

OFILES			= args.o      indent.o   io.o    lexi.o \
			  mbctools.o  parse.o    pr_comment.o

LIBS			= -lld

.include <${RULES_MK}>
