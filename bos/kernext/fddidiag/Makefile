# @(#)70  1.10 src/bos/kernext/fddidiag/Makefile, diagddfddi, bos41J, 9522A_all  5/30/95  18:50:52
#
#   COMPONENT_NAME: FDDIDIAG
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993,1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

OTHERS          = fddi_comio_errids.h   \
		  root/devices.mca.8ef4.diag.err.S \
		  root/devices.mca.8ef4.diag.trc.S

KERNEL_EXT 		= fddidiag fddidiagpin

IDIR 			= /usr/lib/drivers/

ILIST                   = fddidiag fddidiagpin

LIBS			= ${KERNEXT_LIBS}

fddidiag_ENTRYPOINT 	= fddidiag_config
DIAG_LDFLAGS            = -L/etc/drivers -L/tmp/DIAGCDRFS/usr/lib/drivers
LDFLAGS_KERNEXT         += ${DIAG_LDFLAGS}

fddidiag_IMPORTS 		= -bI:fddidiagpin.exp
fddidiagpin_IMPORTS 	= -bI:fddidiag.exp

fddidiag_EXPORTS 		= -bE:fddidiag.exp
fddidiagpin_EXPORTS 	= -bE:fddidiagpin.exp

${fddidiag_OFILES}: fddi_comio_errids.h

fddidiag_OFILES = \
	fddiioctl_t.o	\
	fddisel_t.o	\
	fddircv_t.o	\
	fddiclose_t.o	\
	fddidd_t.o	\
	fddiopen_t.o	\
	fddidiag_t.o	\
	fdditx_t.o

fddidiagpin_OFILES = \
	fddiact_b.o	\
	fddiep_b.o	\
	fddiintr_b.o	\
	fdditx_b.o	\
	fddircv_b.o	\
	fddiprim_b.o	\
	fddinrm_b.o	\
	fddidiag_b.o

fddi_comio_errids.h: devices.mca.8ef4.diag.err.S
		${CP} ${devices.mca.8ef4.diag.err.S:P} .
		${ERRTMPLTBLD} -d devices.mca.8ef4.diag.err.S -o fddi_comio_errids.h
		${RM} -f devices.mca.8ef4.diag.err.S

root/devices.mca.8ef4.diag.err.S: devices.mca.8ef4.diag.err.S
		-${MKDIR} root
		${CP} ${devices.mca.8ef4.diag.err.S:P} root/devices.mca.8ef4.diag.err.S

root/devices.mca.8ef4.diag.trc.S: devices.mca.8ef4.diag.trc.S
		-${MKDIR} root
		${CP} ${devices.mca.8ef4.diag.trc.S:P} root/devices.mca.8ef4.diag.trc.S

.include <${RULES_MK}>

