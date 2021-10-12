# @(#)74        1.8  src/bos/etc/security/Makefile, cfgsauth, bos411, 9428A410j 4/13/94 13:09:47
#
#   COMPONENT_NAME: CFGSAUTH
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1988, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

SUBDIRS		= audit migration
SCRIPTS		= passwd group user lastlog limits environ \
		  login.cfg .profile portlog

ILIST		= ${SCRIPTS}
IDIR		= /etc/security/

NOUPDATE_LIST 	= /etc/security/passwd \
		  /etc/security/group \
		  /etc/security/user \
		  /etc/security/lastlog \
		  /etc/security/limits \
		  /etc/security/environ \
		  /etc/security/login.cfg \
		  /etc/security/.profile \
		  /etc/security/portlog

.include <${RULES_MK}>
