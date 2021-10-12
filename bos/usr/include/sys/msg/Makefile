# @(#)84	1.2  src/bos/usr/include/sys/msg/Makefile, Makefiles, bos411, 9428A410j 2/18/93 11:14:05
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 10,27
#
# IBM CONFIDENTIAL -- (IBM Confidential	Restricted when
# combined with	the aggregated modules for this	product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT	International Business Machines	Corp. 1988, 1989
# All Rights Reserved
#
# US Government	Users Restricted Rights	- Use, duplication or
# disclosure restricted	by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		=${MAKETOP}/bos/kernel/sys/msg

IDIR		= /usr/include/sys/msg/
MSGHDRS		= unix_msg.h
CATFILES	= unix.cat

DATAFILES	= unix_msg.h
INCLUDES	= ${DATAFILES}

.include <${RULES_MK}>

unix.msg: unixset.msg syspfs.msg
	@${RM} -f $@
	${CAT} ${unixset.msg:P} ${syspfs.msg:P} > ${.TARGET}


