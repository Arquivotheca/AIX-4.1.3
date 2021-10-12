# @(#)99	1.5  src/bos/kernext/dlc/errlg/Makefile, sysxdlcg, bos411, 9428A410j 9/2/93 16:15:25
# COMPONENT_NAME: SYSXDLCG   error logging and reporting facility
#
# FUNCTIONS: Makefile for errlog to create .h files
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.


#
# The include files "sysx/dlc/inc/lanerr.h", "sysx/dlc/inc/sdlerr.h",
# and "sysx/dlc/inc/qllcerr.h" are  generated from the
# description files "sysx/dlc/errlg/bosext2.dlctoken.obj.err",
# "sysx/dlc/errlg/bosext2.dlcsdlc.obj.err", and "sysx/dlc/errlg/qllc.err
# respectively.  These file contains the templates for error log entries,
# and are indexed by a unique 32 bit number called the error id.
# Device drivers and commands that log errors log errors by logging this
# error id.
# The list of error ids in the .h files are needed by the dlcs for a build.
# In the past, both files (*.err and *.h) were sget and sput.
# Now, only the *.err files are sput. Programs will expect the
# *.h include files to be created at build time.
# The errtmpltbld command will generate the header files in the current
# directory and then they are linked to the appropriate directory for build.
# errtmpltbld is in the tools directory.
#
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

lan.err_IDIR = /usr/lpp/bos.dlc/com/
qllc.err_IDIR = /usr/lpp/bos.dlc/qllc/
sdl.err_IDIR = /usr/lpp/bos.dlc/sdlc/

EXPDIR= /kernel/include/sys/

LIBS = ${KERNEXT_LIBS}

INCLUDES= lanerr.h sdlerr.h qllcerr.h

ILIST = lan.err qllc.err sdl.err

.include <${RULES_MK}>

lanerr.h : lan.err
	${ERRTMPLTBLD} -p -d ${lan.err:P} -o ${.TARGET}
	
sdlerr.h : sdl.err
	${ERRTMPLTBLD} -p -d ${sdl.err:P} -o ${.TARGET}
	
qllcerr.h : qllc.err
	${ERRTMPLTBLD} -p -d ${qllc.err:P} -o ${.TARGET}
	
