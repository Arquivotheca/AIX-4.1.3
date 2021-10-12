# @(#)33        1.11  src/bos/kernext/ient/Makefile, sysxient, bos41J, 9522A_all 5/30/95 18:51:01
#
# COMPONENT_NAME: sysxient - Integrated Ethernet Device Driver
#
# FUNCTIONS: Makefile for Integrated Ethernet Device Driver
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

OTHERS	= i_enterrids.h  \
	  root/devices.mca.8ef2.com.err.S  \
	  root/devices.mca.8ef2.com.trc.S

VPATH	:=${VPATH}${MAKETOP}bos/kernext/ientdiag

KERNEL_EXT	= ethdd

IDIR	= /usr/lib/drivers/

ILIST	= ethdd

LIBS	= ${KERNEXT_LIBS}

ethdd_ENTRYPOINT	= ient_config

${ethdd_OFILES}: i_enterrids.h

ethdd_OFILES = 	i_entopen.o i_entinit.o i_entcfg.o i_entclose.o \
		i_entutil.o quickrev.o i_entctl.o i_entout.o \
		i_entintr.o i_enterr.o i_entdump.o

i_enterrids.h: devices.mca.8ef2.com.err.S
		${CP} ${devices.mca.8ef2.com.err.S:P} .
		${ERRTMPLTBLD} -d devices.mca.8ef2.com.err.S -o i_enterrids.h
		${RM} -f devices.mca.8ef2.com.err.S

root/devices.mca.8ef2.com.err.S: devices.mca.8ef2.com.err.S
	-${MKDIR} root
	${CP} ${devices.mca.8ef2.com.err.S:P} root/devices.mca.8ef2.com.err.S

root/devices.mca.8ef2.com.trc.S: devices.mca.8ef2.com.trc.S
	-${MKDIR} root
	${CP} ${devices.mca.8ef2.com.trc.S:P} root/devices.mca.8ef2.com.trc.S

.include <${RULES_MK}>
