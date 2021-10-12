#  @(#)03	1.4  src/bos/diag/tu/ped4/Makefile, tu_ped4, bos411, 9428A410j 2/22/94 19:10:11 
#
#   COMPONENT_NAME: TU_PED4
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


LIBRARIES =  libtu_ped4.a 

EXPLIB_TARGETS  = export_libtu_ped4.a
EXPDIR		= /usr/lib/

# To use dbx debugger, un-comment the following 3 lines:
#OPT_LEVEL = 	-g
#CFLAGS = 	-D_BSD -qdbxextra
#STRIP_FLAG = 	

OFILES = exectu.o \
	dd_interface.o \
	load_coff.o \
	host_tests.o \
	host_bim_dma.o \
	h_bim_intr.o

SUBDIRS	= mcode_binary


.include <${RULES_MK}>
