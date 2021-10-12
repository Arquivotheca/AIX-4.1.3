# @(#)15 1.2 src/bos/usr/lib/methods/cfgsys_p/Makefile, cfgmethods, bos41J, 9511A_all 3/14/95 03:30:18
#
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 83
#
# LEVEL 1, 5 Years Bull Confidential Information
#

VPATH		=../common:../graphics:../cfgsys

#CFLAGS		=-DCFGDEBUG			

PROGRAMS	= cfgsys_p

OFILES		= cfgsys_p.o cfgmem_p.o cfgcabinet_p.o cfgdrawer_p.o cfgcpu.o cfgtools.o setvar.o \
		cfg_graphics_60x_frs_tools.o cfg_graphics_frs_tools.o

INCFLAGS	= -I../common -I../cfgsys -I../graphics \
		  -I${MAKETOP}../export/${target_machine}${MP}/kernel/include \
		  -I${MAKETOP}/bos/kernext/disp/inc \
		  -I${MAKETOP}/bos/kernel/io/machdd

ILIST		= cfgsys_p
IDIR		= /usr/lib/methods/

LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
