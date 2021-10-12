# @(#)73        1.5 3/23/94 src/bos/usr/include/graphics/bbl/Makefile, bbldd, bos411, 9428A410j 13:20:28
#
# COMPONENT_NAME: (bbldd) BBL Device Driver
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#

VPATH           = ${MAKETOP}bos/kernext/disp/bbl/bblmacro:${MAKETOP}bos/kernext/disp/bbl/inc

EXPDIR		= /usr/include/graphics/
gs_bbl_trcfmt.s_EXPDIR = /usr/include/graphics/trace/
bbldds.h_EXPDIR	= /usr/include/

DATAFILES     =	bbl.h bbl_ramdac_regs.h bbl_rasterizer_regs.h \
		bbl_trace.h gs_bbl_trcfmt.s bbl_ddf.h bbl_diag_req.h \
		bbldds.h bbl_bd_regs.h

INCLUDES	= ${DATAFILES} 

.include <${RULES_MK}>
