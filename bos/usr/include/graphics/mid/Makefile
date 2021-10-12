# @(#)75        1.5  src/bos/usr/include/graphics/mid/Makefile, peddd, bos411, 9428A410j 3/23/94 15:37:01
# COMPONENT_NAME: (peddd) Pedernales Device Driver
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#

VPATH           =${MAKETOP}bos/kernext/disp/ped/pedmacro:${MAKETOP}/bos/kernext/inc/POWER

EXPDIR          = /usr/include/mid/
ddsmid.h_EXPDIR = /usr/include/

PEDMACRO_FILES  = hw_FIF3dm1_m2.h hw_dd_io_trace.h hw_defer.h hw_hooks.h       \
	        hw_timer.h hw_2dm1con.h hw_FIF2dm1.h hw_FIF3dm1.h hw_FIF3dm2.h \
	        hw_FIFrms.h hw_HCRkern.h hw_PCBrms.h hw_addrs.h hw_dsp.h       \
	        hw_errno.h hw_ind_mac.h hw_macros.h hw_model.h hw_model_rms.h  \
	        hw_names.h hw_regs_k.h hw_regs_u.h hw_rmscon.h hw_se_types.h   \
	        hw_seops.h hw_trace.h hw_typdefs.h hw_locks.h hw_io_kproc.h    \
	        hw_iodefs.h
	
#
#       ddsmid.h is needed by cfg_mid.
#
CFG_FILES       = ddsmid.h

INCLUDES        = ${CFG_FILES} mid.h

.include <${RULES_MK}>
