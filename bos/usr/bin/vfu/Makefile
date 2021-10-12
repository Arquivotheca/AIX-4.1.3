# @(#)79 1.3 src/bos/usr/bin/vfu/Makefile, cmdpios, bos411, 9437C411a 9/16/94 08:05:10
#
# COMPONENT_NAME: (CMDPIOS) For PR-88-VFU printer
#
# FUNCTIONS:
#
# ORIGINS: 83
#
# LEVEL 1, 5 Years Bull Confidential Information
#

CFLAGS = -DBULL_PRG
vfu_LDFLAGS = -s


OFILES= vfu_screen.o\
	vfu_master.o\
	vfu_scrollbar.o\
	vfu_tools.o

LIBS= -lxcurses

# Listes de description:

PROGRAMS= vfu

CLEANFILES= ${PROGRAMS} ${OFILES}

DATAFILES= vfupublic_88

ILIST= ${DATAFILES} vfu
vfu_IDIR=/usr/bin/
vfupublic_88_IDIR= /usr/lib/lpd/pio/bull_vfu/

vfu_IMODE= 755
vfupublic_88_IMODE= 444

.include <${RULES_MK}>
