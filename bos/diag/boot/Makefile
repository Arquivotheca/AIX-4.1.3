# @(#)21	1.3  src/bos/diag/boot/Makefile, diagboot, bos41B, bai4 1/18/95 09:45:09
#
# COMPONENT_NAME: DIAGBOOT
#
# FUNCTIONS: Build executable for the boot environment
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
#
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
# All Rights Reserved
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


PROGRAMS	= cfgmgr chkdskt mergeodm eject

LIBS		= -lodm -lcfg -lc -ldiag

ILIST		= cfgmgr chkdskt mergeodm eject
IDIR		= /usr/lpp/diagnostics/bin/

cfgmgr_OFILES	= cfgmgr.o
chkdskt_OFILES	= chkdskt.o
mergeodm_OFILES = mergeodm.o
eject_OFILES	= eject.o
NOUPDATE_LIST = /usr/lpp/diagnostics/bin/cfgmgr \
		/usr/lpp/diagnostics/bin/chkdskt \
		/usr/lpp/diagnostics/bin/eject \
		/usr/lpp/diagnostics/bin/mergeodm

.include <${RULES_MK}>
