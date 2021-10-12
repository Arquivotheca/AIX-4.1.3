# @(#)31        1.15  src/bos/kernext/disp/ped/Makefile, peddd, bos41J 3/16/95 12:16:44
#
#   COMPONENT_NAME: PEDDD
#
#   FUNCTIONS:
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1989,1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}/bos/kernext/Kernext.mk>

EXPINC_SUBDIRS  = pedmacro inc

SUBDIRS         = cdd

KERNEL_EXT      = middd_load middd_loadpin
IDIR            = /usr/lib/drivers/
ILIST           = middd_load middd_loadpin

VPATH           +=:./config:./ddf:./diag:./dma:./font:./intr:./ksr:./rcm:./pedmacro

INCFLAGS     += -Iinc                                                   \
	        -Ipedmacro                                              \
	        -Iintr							\
		-Ircm


LOCALCFLAGS  =  -DD_PROTECT             \
	        -DITOF                  \
	        -DUSE_R3_REGIONS        \
	        -DMID_DD		\
		-D_RCM

LIBS         =  ${KERNEXT_LIBS}         \
	        librts.a


middd_load_ENTRYPOINT   = mid_config
middd_loadpin_IMPORTS   = middd_load.exp
middd_load_IMPORTS      = middd_loadpin.exp
middd_loadpin_EXPORTS   = -bE:middd_loadpin.exp
middd_load_EXPORTS      = -bE:middd_load.exp

middd_load_OFILES       = mid_config.o

middd_loadpin_OFILES    = midcfl.o midcrsr.o midscr.o midtext.o miducod.o     \
	                  midinit.o midPOStest.o mid_init_ddf.o cfg_qvpd.o    \
	                  mid_close.o mid_ioctl.o cfg_term.o cfg_init.o       \
	                  mid_open.o memtrace.o mid_dev.o memcpy64.o          \
	                  midcfgvt.o midevent.o midddf.o mid_get_correlator.o \
	                  mid_getcpos.o mid_getcolor.o mid_gettextfontindex.o \
	                  mid_getcondition.o mid_swapbuffers.o midexch.o      \
	                  middiag.o miderr.o middma.o midfont.o dsp_pinctx.o  \
	                  intr_switch_done.o intr_fifo.o intr_dsp.o           \
	                  dsp_pick.o dsp_newctx.o fontintr.o intrcolor.o      \
	                  intrcpos.o intrindex.o midintr.o intrcond.o         \
	                  intrevent.o intrswap.o midgp.o defaultrcx.o         \
	                  mid_ctxdma.o midswitch.o midcontext.o midgeom.o     \
	                  midwidin.o midbind.o midpixi.o get_WID_for_PI.o     \
	                  mid_delete_WID.o steal_WID.o get_WID_for_render.o   \
	                  get_unused_WID.o release_guarded_WID.o              \
	                  hw_opname.o hw_trace.o midwatt.o 	              \
	                  mid_getstruct.o mid_endrender.o intrendr.o          \
	                  hw_timer.o

.include <${RULES_MK}>
