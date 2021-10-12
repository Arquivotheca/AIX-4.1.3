# @(#)55        1.2  src/bos/usr/ccs/lib/libbsd/profiled/Makefile, libbsd, bos41J, 9508A 1/30/95 16:06:48
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 10,27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ..

SHARED_LIBRARIES	= libbsd.a
MSGHDRS         = libbsd_msg.h
CATFILES        = libbsd.cat
HFILES          = ${MSGHDRS}

EXPLIB_TARGETS	= export_libbsd.a

ILIST           = libbsd.a
IDIR            = /usr/ccs/lib/libp/

LIBS=libc.a ${PROF_LIBS}
EXPORTS= -bE:libbsd.exp

CFLAGS=	-U_AIX -D_BSD -p

SHARED_OFILES= \
	fcntl.o flock.o ftime.o gcd.o ioctl.o madd.o mdiv.o \
	mktemp.o mout.o msqrt.o mult.o nice.o nlist.o pow.o \
	regex.o getpgrp.o setpgrp.o signal.o sigvec.o sleep.o timezone.o \
	util.o valloc.o vfork.o vtimes.o
	
	

.include <${RULES_MK}>

util.o: libbsd_msg.h
