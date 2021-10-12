# @(#)94	1.8  src/bos/kernext/aixif/Makefile, Makefiles, bos411, 9428A410j 3/14/94 09:28:50
#
#   COMPONENT_NAME: SYSXAIXIF
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1989,1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
.include <${MAKETOP}bos/kernext/Kernext.mk>
.include <${MAKETOP}bos/kernel/net/Net.mk>

KERNEL_EXT	= if_en if_tr if_op if_fd if_ca if_sl slip
IDIR		= /usr/lib/drivers/
ILIST		= if_en if_tr if_op if_fd if_ca if_sl slip

LIBS		= ${KERNEXT_LIBS}

if_en_ENTRYPOINT	= config_en
if_tr_ENTRYPOINT	= config_ie5
if_op_ENTRYPOINT	= config_op
if_fd_ENTRYPOINT	= config_fd
if_ca_ENTRYPOINT	= config_ca
if_sl_ENTRYPOINT	= config_sl
slip_ENTRYPOINT		= config_slip

if_en_EXPORTS		= -bE:if_en.exp
if_en_IMPORTS		= -bI:../inet/netinet.exp
if_tr_EXPORTS		= -bE:if_tr.exp
if_tr_IMPORTS		= -bI:../inet/netinet.exp
if_op_EXPORTS		= -bE:if_op.exp
if_op_IMPORTS		= -bI:../inet/netinet.exp
if_fd_EXPORTS		= -bE:if_fd.exp
if_fd_IMPORTS		= -bI:../inet/netinet.exp
if_ca_EXPORTS		= -bE:if_ca.exp
if_sl_IMPORTS		= -bI:statcmd.exp
if_sl_EXPORTS		= -bE:if_sl.exp
slip_IMPORTS		= -bI:../exp/pse.exp -bI:if_sl.exp
slip_EXPORTS		= -bE:slip.exp

INCFLAGS	=  -I..

if_tr_OFILES = if_ie5.o

.include <${RULES_MK}>
