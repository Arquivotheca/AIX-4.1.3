# @(#)66	1.6  src/bos/usr/bin/uucp/uucpadm/Makefile, cmduucp, bos411, 9428A410j 2/21/94 08:41:32
# COMPONENT_NAME: CMDUUCP
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= uucpadm
MSGHDRS		=
CATFILES	=
HFILES		= ${MSGHDRS}

CFLAGS		+= ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURSESLIBFLAG}

ILIST		= uucpadm uucpadm.hf
IDIR		= /usr/sbin/uucp/

OFILES		= bbase.o bdel.o bins.o bline.o bload.o bmenu.o bstore.o \
		  bupd.o build.o cleanu.o cypher.o cypherp.o cyphert.o \
		  dactive.o dhelp.o derror.o dfault.o dmenu.o do_devices.o \
		  do_dialc.o do_perms.o do_poll.o do_systems.o do_menu.o \
		  do_vi.o input.o memclr.o uucpadm.o prhelp.o srch.o strfcpy.o \
		  strrfcpy.o structs.o update.o zaps.o do_dialers.o \
		  do_sysfiles.o search.o spec_files.o specbase.o sysfiles.o

LIBS		= -lcurses 

.include <${RULES_MK}>
