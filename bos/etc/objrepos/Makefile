# @(#)55        1.45  src/bos/etc/objrepos/Makefile, cfgmethods, bos41J, 9523B_all 5/26/95 18:38:26
#
# COMPONENT_NAME: CFGMETHODS
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 1, 5 Years Bull Confidential Information
#

VPATH		 = ${MAKETOP}/bos/objclass
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/bin/netls

SUBDIRS		= swvpd

CRE_LIST	= srcobj.cre smit_class.cre cfgodm.cre diagodm.cre ILS.cre \
		  slips.cre userprofiles.cre

ADD_LIST	= ILS.add logvol.add lvm.add qdaemon.add \
		  cfgrule.pty.add cfgrule.lvm.add cfgrule.aio.add \
		  aio.add sm_aio.add pty.add \
		  chtz.add cmdarch.add cmdcntl.add cmdcomm.add cmddate.add \
		  cmdfs.add cmdiconv.add cmdmsg.add cmdoper.add \
		  cmdps.add cmdque.add cmdsrc.add cmduname.add netls.add \
		  sm_bosmenu.add \
		  sm_dev.add sm_tty.add \
		  sm_lvm.add sm_printer.add \
		  sm_pty.add sm_sys.add \
		  sm_console.add \
		  swvpd.add usrgrp.add sm_alog.add base_dump.add \
		  base_errlg.add sm_mle.add tcb.add \
                  ${${TARGET_MACHINE}_ADD_LIST}

BUILD_LIST	= SRCsubsys SRCnotify SRCsubsvr sm_menu_opt sm_menu_opt.vc \
                  sm_name_hdr sm_name_hdr.vc sm_cmd_hdr sm_cmd_hdr.vc sm_cmd_opt sm_cmd_opt.vc \
		  PdDv PdDv.vc PdAt PdAt.vc PdCn Config_Rules CuDv CuAt CuAt.vc CuDep CuDvDr \
		  CuVPD DSMenu DSMOptions PDiagDev PDiagAtt CDiagDev TMInput \
		  MenuGoal DAVars FRUB FRUs CC MESSAGES KEYBOARD FONT \
		  slips userprofiles

BUILD_ODM	= yes

OTHERS		= cfgodm.ipl cfgodm.fdb

ILIST		= ${OTHERS} DSMOptions.vc PDiagDev.vc PDiagAtt.vc \
			 ${BUILD_LIST} CC.vc MESSAGES.vc KEYBOARD.vc FONT.vc

IDIR		= /etc/objrepos/
cfgodm.ipl_IDIR = /usr/lib/
cfgodm.fdb_IDIR = /etc/

DSMOptions_IDIR		= /usr/lib/objrepos/
DSMOptions.vc_IDIR	= /usr/lib/objrepos/
DSMenu_IDIR		= /usr/lib/objrepos/
PDiagDev_IDIR		= /usr/lib/objrepos/
PDiagDev.vc_IDIR	= /usr/lib/objrepos/
PDiagAtt_IDIR		= /usr/lib/objrepos/
PDiagAtt.vc_IDIR	= /usr/lib/objrepos/
PdAt_IDIR		= /usr/lib/objrepos/
PdAt.vc_IDIR		= /usr/lib/objrepos/
PdCn_IDIR		= /usr/lib/objrepos/
PdDv_IDIR		= /usr/lib/objrepos/
PdDv.vc_IDIR		= /usr/lib/objrepos/
CC.vc_IDIR		= /usr/lib/objrepos/
MESSAGES.vc_IDIR	= /usr/lib/objrepos/
KEYBOARD.vc_IDIR	= /usr/lib/objrepos/
FONT.vc_IDIR		= /usr/lib/objrepos/
CC_IDIR			= /usr/lib/objrepos/
MESSAGES_IDIR		= /usr/lib/objrepos/
KEYBOARD_IDIR		= /usr/lib/objrepos/
FONT_IDIR		= /usr/lib/objrepos/
sm_cmd_hdr_IDIR         = /usr/lib/objrepos/
sm_cmd_hdr.vc_IDIR      = /usr/lib/objrepos/
sm_cmd_opt_IDIR         = /usr/lib/objrepos/
sm_cmd_opt.vc_IDIR      = /usr/lib/objrepos/
sm_menu_opt_IDIR        = /usr/lib/objrepos/
sm_menu_opt.vc_IDIR     = /usr/lib/objrepos/
sm_name_hdr_IDIR        = /usr/lib/objrepos/
sm_name_hdr.vc_IDIR     = /usr/lib/objrepos/

NOUPDATE_LIST		= ${ILIST:@TMP@${${TMP}_IDIR:U${IDIR}}${TMP}@}

.include "${TARGET_MACHINE}/machdep.mk"

.include <${RULES_MK}>


cfgodm.ipl:	cfgodm.cre
		${RM} -f ${.TARGET}
		${CC} -E ${cfgodm.cre:P} > ${.TARGET}

cfgodm.fdb:	cfgodm.cre2
		${RM} -f ${.TARGET}
		${CC} -E ${cfgodm.cre2:P} > ${.TARGET}

