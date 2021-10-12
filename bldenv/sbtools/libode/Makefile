# @(#)70        1.1  src/bldenv/sbtools/libode/Makefile, bldprocess, bos412, GOLDA411a 1/19/94 17:40:38
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
# Revision 1.11.11.1  1993/11/05  20:34:16  damon
# 	CR 463. Pedantic changes
# 	[1993/11/05  20:33:32  damon]
#
# Revision 1.11.4.11  1993/05/27  18:59:18  marty
# 	CR # 558 - Don't think mklinks() is used anymore.
# 	[1993/05/27  18:58:01  marty]
# 
# Revision 1.11.4.10  1993/05/17  18:23:49  damon
# 	CR 531. move builddata.o and misc.o to PARTIAL build
# 	[1993/05/17  18:22:53  damon]
# 
# Revision 1.11.4.9  1993/04/29  17:40:56  damon
# 	CR 463. Added EXTRA_WARNINGS_CFLAGS
# 	[1993/04/29  17:38:29  damon]
# 
# Revision 1.11.4.8  1993/04/09  14:02:45  damon
# 	CR 417. Added project routines
# 	[1993/04/09  14:02:15  damon]
# 
# Revision 1.11.4.7  1993/03/15  17:47:45  damon
# 	CR 443. Added opentemp.o
# 	[1993/03/15  17:45:11  damon]
# 
# Revision 1.11.4.6  1993/03/04  21:29:48  damon
# 	CR 436. Added sci_client.o
# 	[1993/03/04  20:00:02  damon]
# 
# Revision 1.11.4.5  1993/02/01  23:20:03  damon
# 	CR 417. Added builddata.c
# 	[1993/02/01  21:49:21  damon]
# 
# Revision 1.11.4.4  1993/01/15  18:53:49  damon
# 	CR 376. Added files that sci_rcs.c was broken into
# 	[1993/01/15  18:53:36  damon]
# 
# Revision 1.11.4.3  1993/01/14  16:59:27  damon
# 	CR 399. Added misc.c
# 	[1993/01/14  16:51:48  damon]
# 
# 	Taken from 2.1.1
# 	[1992/06/15  16:29:22  damon]
# 
# Revision 1.11.4.2  1993/01/13  20:31:28  damon
# 	CR 392. Moved lockcmds to lib/libode
# 	[1993/01/13  20:31:01  damon]
# 
# Revision 1.11.2.13  1992/12/21  16:10:33  hester
# 	Added NO_ANSI_CC to CFLAGS.
# 	[1992/12/21  16:10:18  hester]
# 
# Revision 1.11.2.12  1992/12/03  17:20:22  damon
# 	ODE 2.2 CR 183. Added CMU notice
# 	[1992/12/03  17:07:50  damon]
# 
# Revision 1.11.2.11  1992/11/13  15:20:22  root
# 	Removed blist_getrec.o which is not currently used
# 	[1992/11/13  15:04:07  root]
# 
# Revision 1.11.2.10  1992/11/12  18:27:45  damon
# 	CR 329. Added NO_ANSI_CC conditional for building cond.o
# 	[1992/11/12  18:11:07  damon]
# 
# Revision 1.11.2.9  1992/10/29  16:45:29  damon
# 	CR 321. Added lock_sb.c
# 	[1992/10/29  16:45:15  damon]
# 
# Revision 1.11.2.8  1992/09/24  19:01:10  gm
# 	CR282: Made more portable to non-BSD systems.
# 	[1992/09/23  18:21:09  gm]
# 
# Revision 1.11.2.7  1992/09/01  19:34:15  damon
# 	CR 240. Added odexm_client.o
# 	[1992/09/01  19:33:58  damon]
# 
# Revision 1.11.2.6  1992/08/07  18:56:11  damon
# 	CR 267. Removed defuncted files
# 	[1992/08/07  18:54:23  damon]
# 
# Revision 1.11.2.5  1992/07/26  17:37:56  gm
# 	Removed -D_BSD.  Changed BUILD_DATE to only run date if building
# 	interface.o.  Cleaned up the OFILES ordering to remove duplicates
# 	and hopefully help in noticing them in the future.
# 	[1992/07/14  17:15:36  gm]
# 
# Revision 1.11.2.4  1992/06/15  18:10:19  damon
# 	Synched with 2.1.1
# 	[1992/06/15  18:04:22  damon]
# 
# Revision 1.11.4.5  1992/03/25  22:47:47  damon
# 	Changes for ui_print_revision
# 	[1992/03/25  21:52:22  damon]
# 
# Revision 1.11.4.4  1992/03/24  17:07:30  damon
# 	Added -DVA_ARGV for runcmd.c
# 	[1992/03/24  17:05:19  damon]
# 
# Revision 1.11.4.3  1992/03/24  00:38:39  damon
# 	Changed porting to ../libode/porting
# 	[1992/03/23  23:19:25  damon]
# 
# Revision 1.11.4.2  1992/03/10  18:12:15  damon
# 	Changes for LBE removal
# 	[1992/03/10  17:59:04  damon]
# 
# Revision 1.11  1991/12/05  21:04:18  devrcs
# 	Added _FREE_ to copyright marker
# 	[91/08/01  08:10:42  mckeen]
# 
# 	First version using library version of SCAPI
# 	[91/07/31  20:56:56  damon]
# 
# 	Changes for Reno make
# 	[91/03/22  16:19:04  mckeen]
# 
# 	Added ${TARGET_FLAGS} for porting.
# 	[91/01/31  14:11:31  damon]
# 
# 	Polish for ODE II 1.0
# 	[91/01/24  16:13:39  mckeen]
# 
# 	Added -DMACHINE=${MACHINE} to CFLAGS
# 	[91/01/19  14:23:32  mckeen]
# 
# 	Removed references to par_cmd_line and rc_file_ext
# 	[91/01/14  09:45:15  randyb]
# 
# 	Cleaned up for project ode
# 	[91/01/09  13:07:34  mckeen]
# 
# 	Added interface.o, sbdata.o, and stubs.o; removed print_usage.o
# 	[91/01/08  12:07:58  randyb]
# 
# 	Added interface.o and match.o
# 	[90/12/10  16:14:28  damon]
# 
# 	Added blist_getrec.o to OFILES for -from option
# 	and multi-track development.
# 	[90/12/07  16:42:53  robert]
# 
# 	Added default_build routine
# 	[90/11/09  16:42:45  randyb]
# 
# Revision 1.9  90/10/07  20:02:40  devrcs
# 	Added pass to compile static libraries against shared ones.
# 	[90/10/04  21:37:59  gm]
# 
# 	Added EndLog Marker.
# 	[90/09/28  20:08:04  gm]
# 
# 	Added USE_STATIC_LIBRARIES.
# 	[90/09/30  22:20:11  gm]
# 
# Revision 1.8  90/09/23  16:41:50  devrcs
# 	Added explicit EXPLIB_TARGETS line.
# 	[90/09/19  17:37:48  gm]
# 
# Revision 1.7  90/08/09  14:22:40  devrcs
# 	Cleanup for export, install and post-freeze.
# 	[90/08/05  13:30:38  gm]
# 
# 	Moved here from usr/local/sdm/lib/libsb.
# 	[90/08/05  12:32:13  gm]
# 
# $EndLog$

# When running the setup script, BOOTSTRAP is set. During this stage,
# only parts of libode are needed. Namely, the PARTIAL_OFILES and the
# PORTING_OFILES. During normal building, all OFILES are used.

VPATH			= porting
LIBRARIES		= libode.a

EXPLIB_TARGETS		= export_libode.a
EXPDIR			= /usr/lib/

CCTYPE			= ansi
CFLAGS			= -D_ALL_SOURCE
INCFLAGS		= -I${MAKETOP}bldenv/sbtools/include

BUILD_DATE		= ${x:L:!/bin/date!}
interface.o_CFLAGS	= ${CFLAGS} -DBUILD_DATE="\"${BUILD_DATE}\""
par_rc_file.o_CFLAGS	= ${CFLAGS} -DMACHINE=\"${MACHINE}\"
runcmd.o_CFLAGS		= ${CFLAGS} -D${${TARGET_MACHINE}_VA_ARGV}
run.o_CFLAGS		= ${CFLAGS} -D${${TARGET_MACHINE}_VA_ARGV}
run_p.o_CFLAGS		= ${CFLAGS} -D${${TARGET_MACHINE}_VA_ARGV}
free_disk_space.o_CFLAGS= ${CFLAGS} ${NO_STATVFS:D-DNO_STATVFS} \
			  ${NO_STATFS:D-DNO_STATFS} ${USE_FRSIZE:D-DUSE_FRSIZE} \
			  ${USE_BSIZE:D-DUSE_BSIZE}
PORTING_OFILES		= environment.o
PARTIAL_OFILES		= abspath.o builddata.o concat.o ode_cond.o error.o\
			  ffilecopy.o filecopy.o getbool.o interface.o\
			  makepath.o match.o misc.o nxtarg.o par_rc_file.o\
			  path.o project.o rm_newline.o run.o runcmd.o \
			  sbdata.o searchp.o skipto.o uquit.o
FULL_OFILES		= copyrights.o def_build.o editor.o\
			  fdate.o fold.o free_disk_space.o getstab.o getstr.o\
			  history.o lockcmds.o lock_sb.o \
			  odexm_client.o opentemp.o prstab.o quit.o run2.o \
			  sci.o sci_client.o sets.o src_ctl_rcs.o stablk.o \
			  stlmatch.o stubs.o
#OFILES			= ${BOOTSTRAP:U${FULL_OFILES}:D}\
#			  ${PARTIAL_OFILES} ${PORTING_OFILES}
OFILES			= ${PARTIAL_OFILES} ${PORTING_OFILES}

.include <${RULES_MK}>

interface.o: ui.h
