# @(#)18        1.14  src/bos/usr/lib/assist/Makefile, cmdassist, bos411, 9440A411d 10/5/94 08:18:00
#
# COMPONENT_NAME:  cmdassist
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

SUBDIRS		= add_files fr_CA

PROGRAMS        = calc_pgspace \
                  set_pgspace

SCRIPTS         = assist.xinitrc \
                  check_cd \
                  exit_assistant \
                  install_pkg \
                  mount_cd \
                  unmount_cd \
                  assist_main \
                  assist_migrate

MSGHDRS		= cmdassist_msg.h 
CATFILES	= cmdassist.cat softlic.cat
HFILES		= ${MSGHDRS}

ILIST           = ${PROGRAMS} sign_license ${SCRIPTS} assist.resources Toronto.bm AIX.bm \
		  softlic.cat

IDIR            = /usr/lib/assist/

Toronto.bm_IDIR = /usr/lib/assist/bitmaps/
AIX.bm_IDIR = /usr/lib/assist/bitmaps/
softlic.cat_IDIR = /usr/lib/nls/msg/C/


INCFLAGS        =

LIBS            = -lodm


OTHERS =
OTHERS += sign_license
sign_license: sign_license.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
