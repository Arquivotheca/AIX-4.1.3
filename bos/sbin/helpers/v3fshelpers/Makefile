# @(#)92        1.8  src/bos/sbin/helpers/v3fshelpers/Makefile, cmdfs, bos411, 9428A410j 12/13/93 08:14:01
# COMPONENT_NAME: CMDFS - filesystem commands
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

EXPINC_SUBDIRS	= libfs
EXPLIB_SUBDIRS	= libfs

PROGRAMS	= v3fshelper
MSGHDRS		= op_namei_msg.h op_check_msg.h op_make_msg.h op_extend_msg.h 
CATFILES	= op_namei.cat op_check.cat op_make.cat op_extend.cat
HFILES		= ${MSGHDRS}

ILIST		= v3fshelper
IDIR		= /sbin/helpers/

INCLUDES	= fsop.h Fs.h
EXPDIR		= /usr/include/

OFILES		= fshelper.o op_statfs.o op_make.o op_namei.o \
		  op_debug.o op_extend.o op_check.o bufpool.o

INCFLAGS	= -Ikernel/include
LIBS		=  -lfs

#
# 	CC_OPT_LEVEL and _LDFLAGS_PROG_ changed for dbx debugging
#
#CC_OPT_LEVEL	= -g
.include <${RULES_MK}>
#_LDFLAGS_PROG_  = -b f -bh:4 -bi -D0 -H512 -b 'loadmap:${.TARGET}.map' -bR:${.TARGET}.rmap
