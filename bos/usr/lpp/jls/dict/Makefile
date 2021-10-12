# @(#)99        1.2  src/bos/usr/lpp/jls/dict/Makefile, libKJI, bos411, 9428A410j 1/29/93 08:04:25
#
# COMPONENT_NAME: (libKJI) Japanese Input Method
#
# FUNCTIONS: JIM dictionaries
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1991, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

DATAFILES	= adjdict \
			ibmbase \
			ibmcnnc \
			ibmzipc \
			usrdict \
			usrdict.org \
			adjdict.org

ILIST		= ${DATAFILES}
IDIR		= /usr/lpp/jls/dict/

.include <${RULES_MK}>

usrdict.org: usrdict
	${CP} $? $@

adjdict.org: adjdict
	${CP} $? $@
