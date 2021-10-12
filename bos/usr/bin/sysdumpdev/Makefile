# @(#)08        1.6  src/bos/usr/bin/sysdumpdev/Makefile, cmddump, bos411, 9428A410j 8/20/93 14:31:39
# COMPONENT_NAME: cmddump
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS        = sysdumpdev sysdumpstart dmpfmt
MSGHDRS         = cmddump_msg.h
SCRIPTS		= cpydmpdskt
CATFILES        = cmddump.cat
HFILES          = ${MSGHDRS}

SUBDIRS		= smutil

dmpfmt_OFILES	= main.o fmt.o opts.o tbl.o xtr.o
sysdumpdev_OFILES = sysdumpdev.o
sysdumpstart_OFILES = sysdumpstart.o
ILIST           = sysdumpdev sysdumpstart dmpfmt ${SCRIPTS}
IDIR            = /usr/bin/
cpydmpdskt_IDIR = /usr/lib/ras/
dmpfmt_IDIR	= /usr/lib/ras/dmprtns/

LIBS            = -lras -lodm -lrts -lcfg

.include <${RULES_MK}>
