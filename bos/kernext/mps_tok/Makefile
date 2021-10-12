#   "@(#)25        1.2  src/bos/kernext/mps_tok/Makefile, sysxmps, bos41J, 9522A_all 5/30/95 18:50:41";
#
#   COMPONENT_NAME: sysxmps
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

.include <${MAKETOP}bos/kernext/Kernext.mk>

OTHERS          = tr_mps_errids.h \
                  root/devices.mca.8fa2.rte.err.S \
                  root/devices.mca.8fa2.rte.trc.S

KERNEL_EXT	= mpsdd

IDIR		= /usr/lib/drivers/

ILIST           = mpsdd 

mpsdd_ENTRYPOINT	= mps_config
LIBS			= ${KERNEXT_LIBS}  

${mpsdd_OFILES}: tr_mps_errids.h

mpsdd_OFILES = \
	mps_util.o mps_cfg.o mps_open.o mps_close.o mps_tx1.o mps_tx2.o   \
        mps_recv.o mps_wdt.o mps_intr.o mps_function.o mps_ctl.o \
        mps_dump.o 

tr_mps_errids.h: mps_tok.err.S
	 ${CP} ${mps_tok.err.S:P} .
	 ${ERRTMPLTBLD} -d mps_tok.err.S -o tr_mps_errids.h
	 ${RM} -f mps_tok.err.S

root/devices.mca.8fa2.rte.err.S: mps_tok.err.S	
		-${MKDIR} root
		 ${CP} ${mps_tok.err.S:P} root/devices.mca.8fa2.rte.err.S

root/devices.mca.8fa2.rte.trc.S: mps_tok.trc.S
		 -${MKDIR} root
		 ${CP} ${mps_tok.trc.S:P} root/devices.mca.8fa2.rte.trc.S

.include <${RULES_MK}>
