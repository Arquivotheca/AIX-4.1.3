# @(#)46        1.3  src/bos/usr/ccs/bin/gprof/Makefile, cmdstat, bos411, 9428A410j 3/24/94 12:09:29
# COMPONENT_NAME: gprof
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1991, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS		= gprof
MSGHDRS                 = gprof_msg.h
CATFILES                = gprof.cat

ILIST			= gprof
IDIR			= /usr/ccs/bin/

CFLAGS			= -DAIXV3AR
HFILES			= ${MSGHDRS} 

OFILES			= gprof.o arcs.o dfn.o lookup.o header.o \
			  calls.o hertz.o printgprof.o printlist.o

LIBS			= -ldemangle -lld

.include <${RULES_MK}>
