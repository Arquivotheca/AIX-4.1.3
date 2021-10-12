# @(#)28        1.5  src/bldenv/make/Makefile, bldprocess, bos412, 9443A412a 10/30/94 06:40:44
#
#   COMPONENT_NAME: BLDPROCESS
#
#   FUNCTIONS: none
#
#   ORIGINS: 27,71
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# @OSF_FREE_COPYRIGHT@
# COPYRIGHT NOTICE
# Copyright (c) 1992, 1991, 1990  
# Open Software Foundation, Inc. 
#  
# Permission is hereby granted to use, copy, modify and freely distribute 
# the software in this file and its documentation for any purpose without 
# fee, provided that the above copyright notice appears in all copies and 
# that both the copyright notice and this permission notice appear in 
# supporting documentation.  Further, provided that the name of Open 
# Software Foundation, Inc. ("OSF") not be used in advertising or 
# publicity pertaining to distribution of the software without prior 
# written permission from OSF.  OSF makes no representations about the 
# suitability of this software for any purpose.  It is provided "as is" 
# without express or implied warranty. 
#
#
# HISTORY
# $Log: Makefile,v $
# Revision 1.2.8.2  1993/04/26  20:19:54  damon
# 	CR 464. Removed compat.o
# 	[1993/04/26  20:19:47  damon]
#
# Revision 1.2.8.1  1993/04/26  20:19:53  damon
# *** Initial Branch Revision ***
#
# Revision 1.2.2.7  1992/12/03  19:04:43  damon
# 	ODE 2.2 CR 346. Expanded copyright
# 	[1992/12/03  18:34:41  damon]
# 
# Revision 1.2.2.6  1992/09/24  19:22:43  gm
# 	CR286: Major improvements to make internals.
# 	[1992/09/24  17:53:04  gm]
# 
# Revision 1.2.2.5  1992/06/16  21:24:28  damon
# 	2.1.1 touch-up
# 	[1992/06/16  21:15:00  damon]
# 
# Revision 1.2.2.4  1992/06/12  00:48:57  damon
# 	Synched with 2.1.1
# 	[1992/06/11  20:00:03  damon]
# 
# Revision 1.2.2.3  1991/12/31  23:18:53  damon
# 	Changed make to use new libode porting directory
# 	[1991/12/31  23:17:50  damon]
# 
# Revision 1.2.2.2  1991/12/30  21:24:58  damon
# 	Transition to libode from libsb
# 	[1991/12/20  21:58:04  damon]
# 	Revision 1.2.6.2  1992/06/11  20:28:47  damon
# 	Synched with 2.1.1
# 
# Revision 1.2.5.3  1992/04/02  15:02:11  damon
# 	Added ui_print_revision and BUILD_DATE
# 	[1992/04/02  15:00:38  damon]
# 
# Revision 1.2.5.2  1992/02/19  20:44:25  damon
# 	Retro-fitted hp700 and sparc ports
# 	[1992/02/19  18:49:02  damon]
# 
# Revision 1.2  1991/12/05  20:41:47  devrcs
# 	Install make
# 	[91/09/26  14:59:37  marty]
# 
# 	add to (rather than redefine) CFLAGS
# 	[91/05/15  15:43:05  ezf]
# 
# 	Added ffs.c as porting option
# 	[91/04/25  14:19:45  mckeen]
# 
# 	Changes for parallel make.
# 	[91/04/21  16:36:24  gm]
# 
# 	Added MACHINE definition.
# 	[91/04/03  15:18:49  damon]
# 
# 	Removed -MD, added checks for NO_STRDUP and NO_SETENV
# 	[91/04/03  11:07:04  damon]
# 
# 	Changes for Reno make
# 	[91/03/22  15:32:57  mckeen]
# 
# $EndLog$

VPATH		=lst.lib:${ARCHIVE_FORMAT}:${MAKETOP}bldenv/sbtools/libode:${MAKETOP}bldenv/sbtools/libode/porting

PROGRAMS	=make

ILIST		=${PROGRAMS}
IDIR		=/bin/

INCFLAGS	=-Ilst.lib -I${MAKETOP}bldenv/sbtools/include
CC_OPT_LEVEL	=-g
BUILD_DATE	=${x:L:!/bin/date!}
main.o_CFLAGS	=${CFLAGS} -DMACHINE\=\"${MACHINE}\" \
		 -DBUILD_DATE="\"${BUILD_DATE}\""
CARGS		=-D__STDC__ -M -UMSG 

HFILES		=make.h
LIBODE_OFILES	=ode_cond.o
PORTING_OFILES	=${NO_FFS:Dffs.o:U} \
		 ${NO_SETENV:Denvironment.o:U} \
		 ${NO_STRDUP:Dstrdup.o:U} \
		 ${NO_STRERROR:Dstrerror.o:U} \
		 ${NO_GETCWD:Dgetcwd.o:U} \
		 ${NO_WAITPID:Dwaitpwd.o:U}
OFILES		=arch.o arch_fmtdep.o buf.o cond.o dir.o hash.o job.o \
		 main.o make.o parse.o str.o suff.o targ.o var.o \
		 lstAlloc.o lstAppend.o lstClose.o lstConcat.o \
		 lstDeQueue.o lstDestroy.o lstDupl.o \
		 lstFindFrom.o lstForEachFrom.o \
		 lstInit.o lstInsert.o lstIsAtEnd.o \
		 lstMember.o lstNext.o lstOpen.o lstRemove.o \
		 ${LIBODE_OFILES} ${PORTING_OFILES}

.include <${RULES_MK}>
