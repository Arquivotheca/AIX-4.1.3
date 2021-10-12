# @(#)83        1.17  src/bos/kernel/Kernel.mk, Makefiles, bos41J, bai15 4/10/95 08:51:02
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernel/Kernel.tocdata.mk>

LITECFLAGS = -qproclocal ${DATA_IN_TOC}
MODEL_FLAGS = -D_RS6K -D_RSPC -D_RS6K_UP_MCA -D_RSPC_UP_PCI

.if defined(MP) && ${MP} == "_603_DEC"
POLO_WF_603_CFLAGS = -D_603_DEC
MODEL_FLAGS += -D_603_DEC
.endif

.if defined(MP) && ${MP} == "_mp_real"
MODEL_FLAGS += -D_RS6K_SMP_MCA
.endif

.if defined(MP) && ${MP} == "_mp"
MODEL_FLAGS += -D_RS6K_SMP_MCA
.endif

CFLAGS  = -D_KERNSYS -D_KERNEL -D_VOPS ${LOCALCFLAGS} \
	${POLO_WF_603_CFLAGS} \
	${LITECFLAGS} ${MODEL_FLAGS} -DPM_SUPPORT
M4FLAGS += ${MODEL_FLAGS}

KERNEL_BUILD=yes
