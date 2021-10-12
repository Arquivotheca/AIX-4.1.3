# @(#)68	1.3  src/bos/usr/bin/file/Makefile, cmdfiles, bos411, 9428A410j 3/15/94 14:56:28
# COMPONENT_NAME: (CMDFILES) commands that manipulate files
#
# FUNCTIONS: makefile
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

PROGRAMS	= file
MSGHDRS		= file_msg.h magic_msg.h
CATFILES	= file.cat magic.cat
HFILES		= ${MSGHDRS}
DATAFILES	= magic

ILIST		= file magic
IDIR		= /usr/bin/

magic_IDIR	= /etc/

LIBS		= -lPW

NOUPDATE_LIST	= /etc/magic

.include <${RULES_MK}>
