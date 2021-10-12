# @(#)17        1.4  src/bos/diag/tu/artic/Makefile, tu_artic, bos411, 9428A410j 4/13/94 16:36:35
#
#   COMPONENT_NAME:  tu_artic
#
#   FUNCTIONS:  Makefile for Portmaster, MPQP, X.25, and Multiport/2 
#               test units.
#
#   ORIGINS:  27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
#   All Rights Reserved
#
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

LIBRARIES	= libartictu.a

EXPLIB_TARGETS	= export_libartictu.a

EXPDIR		= /usr/lib/

OFILES          = crc.o ddint.o diagld.o exectu.o gettype.o mktu_rc.o \
                  out_br.o rcp_stat.o rw_io.o rw_mem.o st_diag.o st_task.o \
	          tu001.o tu003.o tu004.o tu005.o tu006.o tu007.o tu008.o \
	          tu010.o tu011.o tu012.o tu013.o tu014.o tu015.o tu016.o \
	          tu018.o tu019.o tu020.o tu021.o tu022.o tu023.o tu024.o \
	          tu025.o tu026.o tu027.o tu028.o tu029.o tu030.o tu032.o \
	          tu033.o tu034.o tu035.o tu036.o tu037.o tu038.o tu039.o \
	          tu040.o tu041.o tu042.o tu043.o tu044.o tu045.o tu046.o \
	          tu047.o tu048.o tu049.o tu050.o \
	          tu051.o tu052.o tu053.o tu054.o tu055.o tu056.o tu057.o \
	          tu058.o tu059.o tu060.o 

CFLAGS          = -D DIAGS
LIBS            = -lc

ILIST		= \
                  f0efd.00.02 \
                  708fd.00.00

IDIR		= /usr/lib/microcode/
.include <${RULES_MK}>
