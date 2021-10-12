# @(#)33        1.3  src/bos/usr/lib/nls/loc/imcompose/Makefile, cmdims, bos412, 9443ct.makefile 10/27/94 23:27:56
#
# COMPONENT_NAME: cmdims
#
# FUNCTIONS: Makefile for imcompose
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

OTHERS = \
	sbcs.imcompose \
	el_GR.imcompose \
	BIM.imcompose \
	CIM.imcompose

IDIR = /usr/lib/nls/loc/
ILIST = $(OTHERS) $(OTHERS:.imcompose=.imcompose.src)

.include <${RULES_MK}>

${OTHERS}: $$(.TARGET:.imcompose=.imcompose.src)
	${KEYCOMP} -c < ${${.TARGET:.imcompose=.imcompose.src}:P} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}
