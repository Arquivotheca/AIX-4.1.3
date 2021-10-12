# @(#)35        1.3  src/bos/usr/include/graphics/sys/Makefile, rcm, bos411, 9428A410j 5/19/94 16:05:36
#
# COMPONENT_NAME: (sysxdisp) Display Sub-System
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#

VPFONT 		= ${MAKETOP}bos/kernext/lft/inc/sys
VPAIXGSC 	= ${MAKETOP}bos/kernext/rcm/inc/sys
VPINPUT		= ${MAKETOP}bos/kernext/inputdd/inc/sys
VPSGA 		= ${MAKETOP}bos/kernext/disp/sga/inc
VPWGA 		= ${MAKETOP}bos/kernext/disp/wga/inc
VPATH 		= ${VPFONT}:${VPAIXGSC}:${VPINPUT}:${VPSGA}:${VPWGA}

EXPDIR		= /usr/include/sys/

SHIP_FILES      =	display.h	font.h		pwr_mgr.h	\
			inputdd.h 	lft_ioctl.h     

EXP_FILES       =	\
        		aixgsc.h 	aixfont.h 	\
			gio.h 		ktsmdds.h \
			sga_reg.h sga_regval.h wga_reg.h wga_regval.h

INCLUDES	= ${EXP_FILES}  ${SHIP_FILES} 

IDIR            = /usr/include/sys/
ILIST           = ${SHIP_FILES}

.include <${RULES_MK}>
