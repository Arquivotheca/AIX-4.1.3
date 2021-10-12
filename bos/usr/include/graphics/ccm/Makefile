# @(#)81	1.5  src/bos/usr/include/graphics/ccm/Makefile, dispccm, bos411, 9428A410j 4/22/94 13:29:49
#
#   COMPONENT_NAME: PEDDD
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPCCM		= ${MAKETOP}/bos/kernext/disp/ccm/inc
VPATH		= ${VPCCM}:${MAKETOP}/bos/kernext/disp/inc

EXPDIR		= /usr/include/
frs_60x_dfa.h_EXPDIR = /usr/include/graphics/
gs_mon.h_EXPDIR = /usr/include/graphics/

#
#	The CCM_EXP_FILES are needed by cfg_mid.
#
CCM_SHIP_FILES	= ccm.h			ccm_macros.h 		\
		  cdd.h 		cdd_intr.h    \
		  cdd_intr_macros.h	cdd_macros.h		frs.h         \
		  frs_display.h		frs_display_macs.h	frs_macs.h    \
		  frs_60x.h		60x_regs.h

CCM_EXP_FILES	= ${CCM_SHIP_FILES}	ccm_trchkid.h 		ccmdd_tools.h

MISC_FILES	= gs_mon.h 		frs_60x_dfa.h

INCLUDES	= ${CCM_EXP_FILES}  ${MISC_FILES}

IDIR            = /usr/include/
ILIST           = ${CCM_SHIP_FILES}  ${MISC_FILES}

.include <${RULES_MK}>
