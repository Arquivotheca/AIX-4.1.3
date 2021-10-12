#
#   COMPONENT_NAME: DSAUETH
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1989,1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= 	${MAKETOP}/bos/diag/tu/eth


PROGRAMS	= uenet 

MSGHDRS		= uenet_msg.h
CATFILES 	= uenet.cat
HFILES		= ${MSGHDRS}

LIBS		= -ldiag -lasl  -lodm -lcfg -lc

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/eth 

ILIST		= ${PROGRAMS} uenet.cat

IDIR		= /usr/lpp/diagnostics/bin/

uenet.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

TUS		=	a_dma.o crc_gen.o d_match.o detrace.o ex_cmd.o \
			exectu.o g_atype.o g_netadd.o i_txbd.o mktu_rc.o \
			r_stat.o rw_cmd.o rw_ctrl.o rw_hio.o rw_par.o rw_pos.o \
			rw_rampg.o rw_smem.o rx_stat.o s_filter.o s_indic.o \
			s_loop.o s_netadd.o s_pause.o s_resume.o setup_rx.o \
			setup_tx.o sr_pos.o sr_regs.o st_eth.o tu001.o \
			tu002.o tu003.o tu004.o tu005.o tu006.o tu007.o \
			tu008.o tu009.o tu010.o tu011.o tu012.o \
			tx_stat.o wait_bd.o wait_mb.o cmd_recd.o



uenet_OFILES	= uenet.o sa_disp_mn.o  ${TUS}

.include <${RULES_MK}>


