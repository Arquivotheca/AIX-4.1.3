# @(#)24	1.22  src/bos/usr/include/sys/POWER/machdep.mk, Makefiles, bos41J 3/29/95 16:51:38
# COMPONENT_NAME: cfgmethods
#
# FUNCTIONS:
#
# ORIGINS: 10,27
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

VPATH		:= ${MAKETOP}/bos/kernel/sys/${TARGET_MACHINE}:${MAKETOP}/bos/kernel/vmm/${TARGET_MACHINE}:${VPATH}

POWER_DATAFILES	= \
       cdli_tokuser.h	\
       cdli_fddiuser.h  \
       cdli_entuser.h	\
       cdrom.h		\
       diagex.h		\
       scsi.h		\
       tmscsi.h		\
       serdasd.h	\
       badisk.h		\
       scdisk.h		\
       scarray.h	\
       tapedd.h		\
       ddconc.h		\
       entuser.h	\
       mdio.h		\
       entdisp.h	\
       eu.h		\
       rs.h		\
       li.h		\
       tokuser.h	\
       io3270.h		\
       mpqp.h		\
       bootrecord.h	\
       x25ddi.h		\
       gswio.h		\
       fp_cpusync.h	\
       soluser.h	\
       fddiuser.h	\
       comlink.m4       \
	catuser.h       \
	vmdisk.h	\
	sys_resource.h	\
	cache.h         \
	systemcfg.h     \
	scsi_scb.h	\
	scb_user.h	\
	ide.h		\
	idecdrom.h

POWER_INCLUDES  = \
	inline.h \
	nvdd.h	\
	overlay.h	\
	vmdefs.h	\
	vmlock.h	\
	vmpfhdata.h	\
	vmpft.h		\
	vmscb.h		\
	vmsys.h		\
	vmxpt.h		\
	vmmacs.h	\
	ppda.h		\
	mpc.h		\
	mplock.h	\
	residual.h	\
	pnp.h \
	pcipnp.h \
	isapnp.h \
	genadpnp.h
