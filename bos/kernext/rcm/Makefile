# @(#)94	1.18  src/bos/kernext/rcm/Makefile, rcm, bos41J, 9520A_all 5/15/95 14:01:06
#
#   COMPONENT_NAME: RCM
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# Uncomment this one to get memory leak checking (and other stuff, too).
# RCMDEBUG=1

.include <${MAKETOP}bos/kernext/Kernext.mk>

EXPINC_SUBDIRS  = inc

KERNEL_EXT	= rcm_load rcm_loadpin
IDIR		= /usr/lib/drivers/
ILIST		= rcm_load rcm_loadpin

LIBS		= ${KERNEXT_LIBS}

rcm_load_ENTRYPOINT	= rcmconfig
rcm_load_EXPORTS	= -bE:rcm_load.exp -bE:../exp/rcm.exp
.ifdef RCMDEBUG
rcm_loadpin_EXPORTS	= -bE:rcm_loadpin.exp -bE:rcm_loadpin_dbg.exp
.else
rcm_loadpin_EXPORTS	= -bE:rcm_loadpin.exp
.endif

.ifdef RCMDEBUG
LOCALCFLAGS	=-DUSE_R3_REGIONS -D_AIXV3_POSTWAIT -D_KERNEL -D_RCM -DU_IOMEM_ATT -DRCMDEBUG
.else
LOCALCFLAGS	=-DUSE_R3_REGIONS -D_AIXV3_POSTWAIT -D_KERNEL -D_RCM -DU_IOMEM_ATT # -DDEBUG
.endif

INCFLAGS        =-I./inc

rcm_load_DEP	= rcm_load.exp 
rcm_loadpin_DEP = rcm_loadpin.exp

KERNEXT_IMPORTS	+= -bI:tty.exp

.ifdef RCMDEBUG
rcm_load_LDFLAGS_KERNEXT= ${LDFLAGS_KERNEXT} -bI:${rcm_loadpin.exp:P} -bI:${rcm_loadpin_dbg.exp:P}
.else
rcm_load_LDFLAGS_KERNEXT= ${LDFLAGS_KERNEXT} -bI:${rcm_loadpin.exp:P}
.endif
rcm_loadpin_LDFLAGS_KERNEXT= ${LDFLAGS_KERNEXT} -bI:${rcm_load.exp:P}

rcm_load_OFILES		= \
        rcminit.o 

rcm_loadpin_OFILES	= \
        gsc.o \
        fontshm.o gscdma.o gscevent.o gscddf.o gscshm.o rcmgp.o rcmbusacc.o \
	rcmlock.o rcmrcx.o rcmswitch.o rcmwin.o rcmhsc.o rcmcmap.o \
	rcmerr.o gsctrace.o gscdpm.o rcmioctl.o rcm_pm.o rcmfunnel.o \
	rcmdebug.o rcmvm_getsrval.o

.include <${RULES_MK}>
