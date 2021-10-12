# @(#)16        1.6  src/bos/kernext/mirror/Makefile, cmdmirror, bos411, 9428A410j 6/9/94 08:12:15
# COMPONENT_NAME: CMDMIRROR: Console mirroring
#
# FUNCTIONS:
#
# ORIGINS: 83
#
#  LEVEL 1, 5 Years Bull Confidential Information
#


.include <${MAKETOP}bos/kernext/Kernext.mk>

KERNEL_EXT              = mirror
IDIR            	= /usr/lib/drivers/pse/

ILIST                  = ${KERNEL_EXT}


KERNEXT_IMPORTS         = sysoverlay.exp kernex.exp syscalls.exp -bI:pse.exp -bI:dead.exp sockets.exp -bI:pse_int.exp

LIBS                    = ${KERNEXT_LIBS}

mirror_ENTRYPOINT       = mirror_config

mirror_OFILES		= mirror.o

mirror_LINTFILES   	= mirror.c

.include <${RULES_MK}>

