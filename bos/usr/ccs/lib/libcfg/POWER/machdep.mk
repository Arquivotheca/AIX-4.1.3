# @(#)30        1.6  src/bos/usr/ccs/lib/libcfg/POWER/machdep.mk, libcfg, bos41J, bai15 4/11/95 15:48:13
# COMPONENT_NAME: (LIBCFG) 
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

POWER_EXPORTS = -bE:cfgmach.exp

POWER_SHARED_OFILES	= br.o resgen.o bt_rs6k.o bt_rspc.o bt_mca.o bt_pci.o bt_isa.o bt_pcmcia.o bt_common.o pt.o leds.o getindex.o

