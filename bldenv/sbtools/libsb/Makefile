# @(#)27        1.3  src/bldenv/sbtools/libsb/Makefile, ade_build, bos412, GOLDA411a 12/7/93 16:35:28
#
#   COMPONENT_NAME: bldprocess
#
#   FUNCTIONS: none
#
#   ORIGINS: 27,71
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# Copyright (c) 1990, 1991, 1992  
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
# ODE 2.1.1

# When running the setup script, BOOTSTRAP is set. During this stage,
# only parts of libode are needed. Namely, the PARTIAL_OFILES and the
# PORTING_OFILES. During normal building, all OFILES are used.

LIBRARIES		= libsb.a

EXPLIB_TARGETS		= export_libsb.a
EXPDIR			= /usr/lib/

CFLAGS			= -D_BSD -DINC_TIME -DSTRRETINT
BUILD_DATE		!= /bin/date
interface.o_CFLAGS	= -DBUILD_DATE="\"${BUILD_DATE}\""
par_rc_file.o_CFLAGS	= ${CFLAGS} -DMACHINE=\"${MACHINE}\"
runcmd.o_CFLAGS		= ${CFLAGS} -D${${TARGET_MACHINE}_VA_ARGV}
run.o_CFLAGS		= ${CFLAGS} -D${${TARGET_MACHINE}_VA_ARGV}
run_p.o_CFLAGS		= ${CFLAGS} -D${${TARGET_MACHINE}_VA_ARGV}
PORTING_OFILES		= environment.o
PARTIAL_OFILES		= abspath.o concat.o errmsg.o filecopy.o ffilecopy.o \
			  getbool.o interface.o makepath.o match.o nxtarg.o \
			  path.o nxtarg.o par_rc_file.o rm_newline.o run.o \
			  runcmd.o salloc.o searchp.o skipto.o uquit.o sbdata.o 
FULL_OFILES		= blist_getrec.o concat.o def_build.o editor.o \
			  errmsg.o fdate.o fold.o getstab.o getstr.o \
			  mklinks.o prstab.o quit.o run2.o sci_rcs.o \
			  sindex.o stablk.o stlmatch.o stubs.o
OFILES			= ${BOOTSTRAP:U${FULL_OFILES}} \
			  ${PARTIAL_OFILES} ${PORTING_OFILES}
.include <${RULES_MK}>

interface.o: ui.h
