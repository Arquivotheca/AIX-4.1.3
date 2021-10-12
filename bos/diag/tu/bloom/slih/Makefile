# @(#)45        1.2  src/bos/diag/tu/bloom/slih/Makefile, tu_bloom, bos41J, 9514A_all 4/3/95 13:48:23
#
# COMPONENT_NAME: (tu_bloom) PCI SCSI Adapter Test Units
#
# FUNCTIONS: Makefile for second level interrupt handler
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
 
.include <${MAKETOP}/bos/kernext/Kernext.mk>

VPATH 			+= ${MAKETOP}/bos/kernext/exp

KERNEL_EXT		= bloom_intr

bloom_intr_ENTRYPOINT	= bloom_interrupt

bloom_intr_IMPORTS	= -bI:diagex.exp

bloom_intr_EXPORTS	= -bE:bloom_intr.exp

bloom_intr_OFILES	= bloom_intr.o 

LIBS			= ${KERNEXT_LIBS} 

INCFLAGS 		+= -I${MAKETOP}/bos/diag/tu/bloom \
			-I${MAKETOP}/rspc/kernext/pci/ncr810 

ILIST			= bloom_intr

IDIR			= /usr/lpp/diagnostics/slih/

.include <${RULES_MK}>
