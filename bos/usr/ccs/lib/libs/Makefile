# @(#)92	1.7  src/bos/usr/ccs/lib/libs/Makefile, libs, bos411, 9428A410j 4/19/94 18:04:35
#
#   COMPONENT_NAME: LIBS
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

LIBRARIES	= libs_nshr.a
MSGHDRS		= libs_msg.h
CATFILES	= libs.cat
HFILES		= ${MSGHDRS}

EXPLIB_TARGETS	= export_libs_nshr.a

IDIR		= /usr/ccs/lib/
YFLAGS		= -d

OFILES=	\
	acl_chg.o acl_get.o acl_put.o acl_set.o auditwrite.o	\
	privilege.o tcb.o newpass.o auditpack.o auditread.o	\
	ckuserID.o ckuseracct.o getconfattr.o getgroupattr.o	\
	getuserattr.o getuserpw.o gettcbattr.o getpenv.o	\
	setpenv.o setpcred.o getpcred.o commonattr.o AFsec.o	\
	libsmsg.o libs_open.o libs_read.o libs_util.o		\
	libs_write.o authgram.g.o authgram.lx.o authenticate.o	\
	chpass.o util.o mb.o pwdbm.o getportattr.o loginlog.o	\
	checktime.o loginrestrictions.o userlicense.o userlicense2.o \
	pw_rest.o pw_hist.o pw_hist2.o


.include <${RULES_MK}>
