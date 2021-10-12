# @(#)41        1.3  src/bos/kernel/ldr/Makefile, sysldr, bos411, 9428A410j 10/27/93 16:31:20
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernel/Kernel.mk>

VPATH		=${TARGET_MACHINE}

PAGED_OFILES_PRIM	=      \
        xmalloc.o

PAGED_OFILES	=	\
	ld_data.o	\
	ld_files.o	\
	ld_libld.o	\
	ld_lookup.o	\
	ld_memory.o	\
	ff_alloc.o

PAGED_OFILES_BACK	=      \
	ld_execld.o	\
	ld_kernld.o	\
	ld_load.o	\
	ld_symbols.o

INIT_OFILES	= \
	kmem_init.o \
	ld_init.o


.include <${RULES_MK}>
