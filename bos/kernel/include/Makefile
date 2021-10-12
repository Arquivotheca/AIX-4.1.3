# @(#)23        1.12  src/bos/kernel/include/Makefile, kernel, bos411, 9428A410j 4/11/94 13:11:21
#
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
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

EXPINC_SUBDIRS	= aixif sys net netinet pse vmm jfs netns netiso

SUBDIRS	= sys

VPATH		=${MAKETOP}bos/usr/include:${MAKETOP}/bos/usr/include/${TARGET_MACHINE}:${MAKETOP}bos/kernel/ldr:${MAKETOP}bos/kernel/ldr/${TARGET_MACHINE}:${MAKETOP}bos/kernext/tty

DATAFILES	= \
	ld_data.h m_ld_data.h toc.h jctype.h aouthdr.h procinfo.h fcntl.h \
	NLchar.h NLctype.h a.out.h ar.h cf.h ctype.h dbug.h dbxstclass.h \
	errno.h execargs.h exceptab.h filehdr.h float.h grp.h limits.h \
	linenum.h loader.h locale.h macros.h memory.h mesg.h mon.h \
	netdb.h nl_types.h nlist.h reloc.h scnhdr.h signal.h standards.h \
	stdarg.h stdio.h stdlib.h storclass.h string.h strings.h syms.h typchk.h \
	uinfo.h ulimit.h unistd.h ustat.h values.h varargs.h xcoff.h \
	stddef.h termios.h lvm.h va_list.h assert.h malloc.h sgtty.h odmi.h  \
	dirent.h fpxcp.h fptrap.h
	
INCLUDES	= ${DATAFILES} mirror.h ldtty.h

EXPDIR		=/kernel/include/


.include <${RULES_MK}>
