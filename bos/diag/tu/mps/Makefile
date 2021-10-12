# static char sccsid[] = "@(#)43        1.2  src/bos/diag/tu/mps/Makefile, tu_mps, bos411, 9437B411a 8/30/94 18:35:08";
#
#   COMPONENT_NAME: tu_mps
#
#   FUNCTIONS: none
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

SUBDIRS         = slih

EXPINC_SUBDIRS  = slih
EXPLIB_SUBDIRS  = slih

LIBRARIES	= libmpstu.a

EXPLIB_TARGETS  = export_libmpstu.a
EXPDIR		= /usr/lib/

OFILES		 = \
        mps_afuncs.o     \
        getmps.o         \
        mps_tools.o      \
        mps_exectu.o     \
        mps_rw_funcs.o   \
        mps_tu000.o      \
        mps_tu001.o      \
        mps_tu002.o      \
        mps_tu003.o      \
        mps_tu004.o      \
        mps_tu005.o      \
        mps_tu006.o      \
        mps_tu007.o      \
        mps_tu010.o

CFLAGS          = -DDIAGPATH

.include <${RULES_MK}>
