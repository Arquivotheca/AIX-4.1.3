# @(#)31        1.2  src/bos/usr/lib/boot/chramfs/Makefile, bosboot, bos411, 9428A410j 8/26/93 15:15:07
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= chramfs
MSGHDRS		=
CATFILES	=
HFILES		= ${MSGHDRS}

ILIST		= chramfs
IDIR		= /usr/lib/boot/
INCFLAGS	=-I${MAKETOP}/bos/src/sbin/helpers/v3fshelpers \
		 -I${MAKETOP}/bos/kernel/vmm/${TARGET_MACHINE} \
		 -I${MAKETOP}/bos/kernel/sys

.include <${RULES_MK}>

