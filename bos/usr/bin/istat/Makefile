# @(#)43        1.2  src/bos/usr/bin/istat/Makefile, cmdfs, bos411, 9428A410j 11/20/92 09:21:31
#
# COMPONENT_NAME: (CMDFS) commands that deal with the file system
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1992
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


PROGRAMS	= istat
MSGHDRS		= istat_msg.h
CATFILES	= istat.cat
HFILES		= ${MSGHDRS}

ILIST		= istat
IDIR		= /usr/bin/
LIBS		= -lfs


#
# for dbx debugging
#
#CC_OPT_LEVEL	= -g
#.include <${RULES_MK}>
#_LDFLAGS_PROG_  = -b f -bh:4 -bi -D0 -H512 -b 'loadmap:${.TARGET}.map' -bR:${.TARGET}.rmap

#
# For no debug
#
.include <${RULES_MK}>

