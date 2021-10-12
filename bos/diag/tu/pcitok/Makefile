# @(#)02	1.2  src/bos/diag/tu/pcitok/Makefile, tu_pcitok, bos41J, 9514A_all 3/30/95 14:04:19
#
#   COMPONENT_NAME: tu_pcitok
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

SUBDIRS         = slih

EXPINC_SUBDIRS  = slih
EXPLIB_SUBDIRS  = slih

LIBRARIES	= libskytu.a

EXPLIB_TARGETS  = export_libskytu.a
EXPDIR		= /usr/lib/

OFILES		 = \
        sky_afuncs.o     \
        getsky.o         \
        sky_tools.o      \
        sky_exectu.o     \
        sky_rw_funcs.o   \
	sky_open.o       \
	sky_init.o       \
	sky_tx.o         \
	sky_rx.o         \
        sky_tu000.o      \
        sky_tu001.o      \
        sky_tu002.o      \
        sky_tu003.o      \
        sky_tu004.o      \
        sky_tu005.o      \
        sky_tu006.o      \
        sky_tu007.o      \
        sky_tu010.o

CFLAGS          = -DDIAGPATH

.include <${RULES_MK}>
