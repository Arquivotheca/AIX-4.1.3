# @(#)16        1.21.5.8  src/bos/kernext/disp/gem/Makefile, sysxdispgem, bos411, 9428A410j 4/29/94 13:57:20
#
#   COMPONENT_NAME: SYSXDISPGEM
#
#   FUNCTIONS: 
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



.include <${MAKETOP}/bos/kernext/Kernext.mk>

EXPINC_SUBDIRS = com

KERNEL_EXT = gemdd
IDIR = /usr/lib/drivers/
ILIST = gemdd

VPATH +=:./rcm:./com

INCFLAGS += -Iinc                                           \
           -Icom

LIBS = ${KERNEXT_LIBS}           \
       librts.a   

gemdd_ENTRYPOINT = gem_config
gemdd_IMPORTS = ipl.exp

gemdd_OFILES =         \
       gem_bind.o      \
       gem_chkdev.o    \
       gem_client_clip.o \
       gem_cwat.o      \
       gem_cwg.o       \
       gem_cxt.o       \
       gem_dd.o        \
       gem_ddfun.o     \
       gem_dma.o       \
       gem_esfbuf.o    \
       gem_esfun.o     \
       gem_eswtch.o    \
       gem_event.o     \
       gem_geom.o      \
       gem_hwid.o      \
       gem_icxt.o      \
       gem_intr.o      \
       gem_ircx.o      \
       gem_mkgp.o      \
       gem_mvcxt.o     \
       gem_obsc.o      \
       gem_rcx.o       \
       gem_rcxp.o      \
       gem_svrs.o      \
       gem_switch.o    \
       gem_trace.o     \
       gem_uwg.o       \
       gem_vdd.o       \
       gem_wind.o      \
       rcmstub.o

#gem_esfun.o_CFLAGS  = -DFAULTHAK
#gem_intr.o_CFLAGS   = -DTIMER 
#gem_eswtch.o_CFLAGS = -DPOLL3D -DPOLL2D
#gem_svrs.o_CFLAGS   = -DNEW_HOTKEY

CFLAGS = -qmaxmem=5000 -DNEW_UCODE -DFAST_LOCK -DUSE_R3_REGIONS -D_KERNEL \
         -DFAULTHAK -DTIMER -DPOLL3D -DPOLL2D -DNEW_HOTKEY -D_RCM

.include <${RULES_MK}>


