# @(#)43	1.3  src/bos/usr/ccs/bin/dump/Makefile, cmdaout, bos411, 9428A410j 3/10/94 10:12:49
# COMPONENT_NAME: CMDAOUT (dump command)
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1991,1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS		= dump
MSGHDRS                 = dump_msg.h
CATFILES                = dump.cat

ILIST			= dump
IDIR			= /usr/sbin/

HFILES			= ${MSGHDRS} 

OFILES			= archive.o dump.o xcoff.o

LIBS			= -lIN -lld 


.include <${RULES_MK}>
