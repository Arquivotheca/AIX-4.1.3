# @(#)94	1.3  src/bos/usr/bin/dosdir/Makefile, cmdpcdos, bos411, 9428A410j 8/4/93 12:56:40
#
# COMPONENT_NAME: CMDPCDOS
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= dosdir

MSGHDRS		= dosdir_msg.h
CATFILES	= dosdir.cat
HFILES		= ${MSGHDRS} pcdos.h tables.h
OFILES		= dosdir.o DFstr.o analyze.o dchdir.o dclose.o dcreate.o \
		  devio.o dfirst.o dlock.o dlseek.o dmount.o dnext.o dopen.o \
		  doschdir.o dosinit.o dospwd.o dread.o dremove.o drmdir.o \
		  dunmount.o dwrite.o getnewcl.o locate.o makedate.o \
		  pause.o putfat.o reclaim.o semafore.o util.o

ILIST		= dosdir
IDIR		= /usr/bin/

.include <${RULES_MK}>
