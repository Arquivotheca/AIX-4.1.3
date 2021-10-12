# @(#)22        1.7  src/bos/kernext/pse/mods/Makefile, Makefiles, bos41J, 9514A_all 4/4/95 15:45:49
#
#   COMPONENT_NAME: SYSXPSE
#
#   FUNCTIONS: none
#
#   ORIGINS: 27,83
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1991,1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# LEVEL 1, 5 Years Bull Confidential Information
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

#CDEBUGFLAGS		= \
#			-DSTREAMS_DEBUG

LOCALCFLAGS 		= ${CDEBUGFLAGS}

LIBS			= ${KERNEXT_LIBS}

OBJECTS			= \
			stdmod.o \
			stddev.o \
			sc.o \
			spx.o \
			echo.o \
			errgen.o \
                        flowmod.o \
                        strnull.o \
                        tmux.o \
                        tmod.o \
                        timod.o \
			tirdwr.o \
			xtiso.o

KERNEL_EXT		= stdmod \
			stddev \
			sc \
			spx \
			echo \
                        errgen \
                        flowmod \
                        strnull \
                        tmux \
                        tmod \
                        timod \
			tirdwr \
			xtiso

IDIR			= /usr/lib/drivers/pse/
ILIST			= ${KERNEL_EXT}

IMPORTS			= pse.exp pse_int.exp sockets.exp statcmd.exp

stdmod_ENTRYPOINT	= stdmod_config
stddev_ENTRYPOINT	= stddev_config
sc_ENTRYPOINT		= sc_config
spx_ENTRYPOINT		= spx_config

echo_ENTRYPOINT         = echo_config
errgen_ENTRYPOINT       = err_config
flowmod_ENTRYPOINT      = flow_config
strnull_ENTRYPOINT      = sn_config
timod_ENTRYPOINT        = timod_config
tmod_ENTRYPOINT         = tmod_config
tmux_ENTRYPOINT         = tmux_config
tirdwr_ENTRYPOINT       = tirdwr_config
xtiso_ENTRYPOINT        = xtiso_config

.include <${RULES_MK}>
