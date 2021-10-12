# @(#)03        1.2  src/bos/diag/tu/lega/Makefile, tu_lega, bos411, 9428A410j 1/12/94 09:33:22
#
#   COMPONENT_NAME: TU_LEGA
#
#   FUNCTIONS:
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

LIBRARIES = libtu_lega.a

EXPLIB_TARGETS	= export_libtu_lega.a
EXPDIR		= /usr/lib/

OFILES = exectu.o \
	dd_interface.o \
	load_coff.o \
	host_tests.o \
	host_bim_dma.o \
	h_bim_intr.o

SUBDIRS = mcode_binary

.include <${RULES_MK}>
