# @(#)33	1.9  src/bos/kernext/x25/Makefile, sysxx25, bos411, 9428A410j 4/1/94 13:21:26
# COMPONENT_NAME:
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

INCLUDES        = x25user.h
x25user.h_IDIR	= /usr/include/sys/

# x25user.h is the publically shipped file. It is created automatically
# from the internal files. x25user.st is used to
# top the information from the js*.h files.
# we echo an endif at the end of the file.
# We need to change the SCCS id in the generated file to show the
# new filename.

x25user.h : x25user.awk x25user.st jsdefs.h xdhx25.h jsxbuf.h
	${CAT} ${x25user.st:P} | ${SED} "s/x25user.st/x25user.h/g" > x25user.h
	${AWK} -f ${x25user.awk:P} ${jsdefs.h:P} ${xdhx25.h:P} ${jsxbuf.h:P} >> x25user.h
	${ECHO} "#endif" >> x25user.h

.include <${RULES_MK}>
