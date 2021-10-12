# @(#)73	1.9  src/bos/usr/bin/ex/tvi/Makefile, cmdedit, bos411, 9428A410j 4/17/94 14:05:12
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 3, 10, 26
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ..

PROGRAMS	= tvi 
MSGHDRS		= ex_msg.h

ILIST		= tvi 
IDIR		= /usr/bin/

# Use these only for debugging.  They should
# not be available to the end user.
# TRACEDEFS	= -DTRACE -DUNDOTRACE -DSUSPTRACE -DMYDEBUG -DADEBUG \
#		  -DMDEBUG -DFDEBUG -DIDEBUG -DRDEBUG

DEFS		= -DSTDIO -DMYSIG -D_SECURITY -DCHDIR -DLISPCODE \
		  -DSINGLE -DTABS=8 -DUSG -DVMUNIX
UNDEFS		= -UFLOCKFILE -UUCBV7 -UCVISUAL -UV6 -UVFORK

INCFLAGS	= -I..

CFLAGS		+= -DTVI ${DEFS} ${UNDEFS} ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURSESLIBFLAG}

tvi_HFILES	= ex.h ex_argv.h ex_re.h ex_temp.h ex_tty.h ex_tune.h \
		  ex_vars.h ex_vis.h ex_msg.h

tvi_OFILES 	= ex.o ex_addr.o ex_cmds.o ex_cmds2.o ex_cmdsub.o ex_data.o \
 		  ex_extern.o ex_get.o  ex_io.o  ex_put.o ex_re.o ex_set.o  \
  		  ex_subr.o ex_temp.o ex_tty.o ex_v.o ex_vadj.o ex_vget.o   \
 		  ex_vmain.o ex_voper.o ex_vops.o ex_vops2.o ex_vops3.o     \
		  ex_vput.o ex_vwind.o printf.o setupterm.o


tvi_LIBS	= -lcurses -ls

.include <${RULES_MK}>

${tvi_OFILES}:	ex_msg.h ${tvi_HFILES}

ex_vars.h: makeoptions ex_data.c
	${CP} ${makeoptions:P} .
	${CHMOD} +x makeoptions
	./makeoptions ${ex_data.c:P} ${_CCFLAGS_}
	${RM} -f makeoptions

