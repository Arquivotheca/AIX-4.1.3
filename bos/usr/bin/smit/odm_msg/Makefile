# @(#)29	1.31 src/bos/usr/bin/smit/odm_msg/Makefile, odm_msg, bos411, 9435A411a 8/29/94 11:42:06
# 
# COMPONENT_NAME: 
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
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
# LEVEL 1, 5 Years Bull Confidential Information
#

SCRIPTS		= smit_no_MSG

INCLUDES	= smit_msg.h smit_cat.h

EXPORTS		= smit_msg.h smit_cat.h

EXPDIR		= /usr/include/

##############################################
#                                            #
# WARNING: .msg files MUST be in set number  #
#       order!!!!                            #
#                                            #
#       (format: component.set_number)       #
#                                            #
# WARNING: be sure to always include a       #
#       "$quote<space><space>" line -- IF    #
#       you don't otherwise set your quote   #
#       character; otherwise gencat will     #
#       (usually) fail when these files are  #
#       combined. ALWAYS set $quote!         #
#                                            #
# WARNING: scgen does not procduce X/open-   #
#       conformaing format.  There should be #
#       EXACTLY 1 space following the        #
#       number and the message.              #
#                                            #
##############################################

#smit_msg_list = 
#	sm_inst.msg	: set 1
#	tcbauth.msg	: set 3
#	tcpip.msg	: set 4
#	dlccfg.msg	: set 5 
#	sm_lft.msg	: set 7 
#	sm_cmderrlg.msg	: set 8
#	sm_cmdtrace.msg	: set 9
#	sm_cmddump.msg	: set 10
#	cmdlvmsmit.msg	: set 15
#	cmdstat.msg	: set 17
#	cmdque.msg	: set 18
#	cmdfs.msg	: set 19
#	cmdcntl.msg	: set 20
#	cmdarch.msg	: set 21
#	cmdoper.msg	: set 22
#	cmdcomm.msg	: set 23
#	x11.msg		: set 24
#	menu.25.msg	: set 25
#	cfgmeth.msg	: set 27
#	sm_cmdps.msg	: set 28
#	cmdlice.msg	: set 29
#	cmdnfs.msg	: set 39
#	cmdtz.msg	: set 30
#	sm_dwm.msg	: set 42
#	cmdsrc.msg	: set 40
#	cmdiconv.msg	: set 43
#	aio.msg		: set 44
#	cmdmsg.msg	: set 44
#	sm_rds.msg	: set 46
#	alog		: set 50
#	sm_netls	: set 51
#       sm_assist.msg	: set 52
#	sm_mle.msg	: set 53
#       sm_assist.msg	: set 54
#       sm_netls2.msg	: set 54
# 	pwrcmd.msg      : set 55
#	smit.98.msg	: set 98
#	smit.99.msg	: set 99
#	smit.99.msg	: set 100
#	smit.101.msg	: set 101
# 	dtsmit.msg 	: set 102

SMIT_MSG_LIST 	= sm_inst.msg sm_tcbauth.msg  \
              	  tcpip.msg dlccfg.msg sm_lft.msg sm_cmderrlg.msg  \
           	  sm_cmdtrace.msg sm_cmddump.msg cmdlvmsmit.msg cmdstat.msg  \
             	  cmdque.msg cmdfs.msg cmdcntl.msg cmdarch.msg  \
            	  cmdoper.msg cmdcomm.msg x11.msg menu.25.msg  \
            	  cfgmeth.msg sm_cmdps.msg cmdlice.msg cmdtz.msg  \
             	  sm_cmdnfs.msg cmdsrc.msg \
           	  cmdiconv.msg cmdmsg.msg sm_rds.msg sm_cmdalog.msg sm_netls.msg \
	          sm_assist.msg sm_mle.msg sm_netls2.msg pwrcmd.msg smit.98.msg \
           	  smit.99.msg smit.101.msg 

MSGHDRS		= smit_msg.h
OTHERS		= smit.msg smit_cat.h ${MSGHDRS}

CATFILES	= smit.cat piosmit.cat dtsmit.cat

.include <${RULES_MK}>

smit.msg:	${SMIT_MSG_LIST}
	${RM} -f ${.TARGET}
	${CAT} ${.ALLSRC} > ${.TARGET}

smit_cat.h:	smit.prolog smit.msg smit_no_MSG
	./smit_no_MSG ${smit.prolog:P} ${smit.msg:P}
