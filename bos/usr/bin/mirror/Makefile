# @(#)11        1.6  src/bos/usr/bin/mirror/Makefile, cmdmirror, bos412, 9445C412a 11/4/94 09:38:45
#
# COMPONENT_NAME:  CMDMIRROR: Console mirroring
#
# FUNCTIONS:
#
# ORIGINS: 83
#
#  LEVEL 1, 5 Years Bull Confidential Information
#

VPATH           = ${MAKETOP}bos/kernext/exp
PROGRAMS        = mirrord
SCRIPTS         = cfgmir
MSGHDRS         = mirrord_msg.h
CATFILES        = mirrord.cat
HFILES          = ${MSGHDRS}
OTHERS          = exp/pse.exp

ILIST           = mirrord cfgmir
mirrord_IDIR    = /usr/sbin/
cfgmir_IDIR     = /usr/sbin/
OFILES          = mirrord.o modem_gr.o

LIBS            = -bI:exp/pse.exp -lodm -lcfg

.include <${RULES_MK}>

${OTHERS} : $${.TARGET:T}
	${RM} -f ${.TARGET}
		[ -d ${.TARGET:H} ] || ${MKDIR} ${.TARGET:H}
			${CP} ${.ALLSRC} ${.TARGET}


