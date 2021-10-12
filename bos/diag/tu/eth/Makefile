# @(#)46	1.3  src/bos/diag/tu/eth/Makefile, tu_eth, bos411, 9428A410j 1/7/94 13:58:53
#
#   COMPONENT_NAME: TU_ETH
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#



#-------------------------------------------------------------------------#
#                                                                         #
#     This Makefile builds the library libethtu.a for the DA application #
#                                                                         #
#-------------------------------------------------------------------------#

LIBRARIES       =   libethtu.a

EXPLIB_TARGETS  =   export_libethtu.a
EXPDIR          =   /usr/lib/

OFILES		= 	a_dma.o crc_gen.o d_match.o detrace.o ex_cmd.o \
			exectu.o g_atype.o g_netadd.o i_txbd.o mktu_rc.o \
			r_stat.o rw_cmd.o rw_ctrl.o rw_hio.o rw_par.o \
			rw_pos.o rw_rampg.o rw_smem.o rx_stat.o s_filter.o \
			s_indic.o s_loop.o s_netadd.o s_pause.o s_resume.o \
			setup_rx.o setup_tx.o sr_pos.o sr_regs.o st_eth.o \
			tu001.o tu002.o tu003.o tu004.o tu005.o tu006.o \
			tu007.o tu008.o tu009.o tu010.o tu011.o tu012.o \
			tx_stat.o wait_bd.o wait_mb.o cmd_recd.o

.include <${RULES_MK}>
