# @(#)90	1.4  src/bos/usr/bin/sccs/Makefile, cmdsccs, bos411, 9428A410j 1/10/94 12:29:39
# COMPONENT_NAME: (CMDSCCS)
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

SUBDIRS		= lib

PROGRAMS	= admin comb delta get sccs sccshelp prs rmdel unget val vc what
SCRIPTS		= sccsdiff
MSGHDRS		= cm_msg.h co_msg.h admin_msg.h comb_msg.h delta_msg.h \
		  get_msg.h sccshelp_msg.h prs_msg.h rmchg_msg.h unget_msg.h \
		  val_msg.h vc_msg.h bsd_msg.h 
CATFILES	= sccs.cat
HFILES		= ${MSGHDRS}
OTHERS		= sccs.msg

ILIST		= ${PROGRAMS} ${SCRIPTS}
IDIR		= /usr/bin/

LIBFLAGS	= -Llib
LIBS		= -lsccs -ls -lPW

.include <${RULES_MK}>

rmdel_OFILES	= rmchg.o

sccs.msg:	sccsdiff.msg cm.msg co.msg admin.msg comb.msg delta.msg get.msg \
		sccshelp.msg prs.msg rmchg.msg unget.msg val.msg vc.msg bsd.msg
	${CAT} ${.ALLSRC} > ${.TARGET}
