# @(#)48        1.3  src/bos/diag/tu/bblue/Makefile, tu_bblue, bos411, 9428A410j 4/28/94 14:34:35
#
#   COMPONENT_NAME: (tu_bblue) Baby Blue Graphics Adapter Test Units
#
#   FUNCTIONS: Makefile
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

VPATH = ${MAKETOP}/bos/kernext/exp

LIBRARIES = libtu_bblue.a

EXPLIB_TARGETS	= export_libtu_bblue.a
EXPDIR		= /usr/lib/

IMPORTS = -bI:rcm.exp -bI:diagex.exp 

CFLAGS = \
	${RS2_601} \
	-D_POWER \
	-DLFT_ACCESS

OFILES = \
	bblue_areafill.o \
	bblue_bd_basic.o \
	bblue_bd_func.o \
	bblue_bd_util.o \
	bblue_bl_adv.o \
	bblue_bl_basic.o \
	bblue_bl_util.o \
	bblue_bltfunc.o \
	bblue_dd_interface.o \
	bblue_dfa.o \
	bblue_dma_int.o \
	bblue_exectu.o \
	bblue_lines.o \
	bblue_qpax_reg.o \
	bblue_qpax_util.o \
	bblue_rom.o \
	bblue_scroll_h.o \
	bblue_svc_aids.o \
	bblue_swizzle.o \
	bblue_util.o \
	bblue_vram.o \
	bblue_windows.o

.include <${RULES_MK}>
