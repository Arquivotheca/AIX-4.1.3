# @(#)00        1.4  src/bos/usr/lib/nim/Makefile, cmdnim, bos41J, 9521B_all 5/24/95 13:08:49
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./lib/nim/Makefile
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

EXPINC_SUBDIRS = objrepos 
EXPLIB_SUBDIRS = lib 
SUBDIRS = objrepos lib methods awk

ILIST	= README
IDIR	= /usr/lpp/bos.sysmgt/nim/

.include <${RULES_MK}>

README: README.S
	${CP} ${README.S:P} ${.TARGET}

