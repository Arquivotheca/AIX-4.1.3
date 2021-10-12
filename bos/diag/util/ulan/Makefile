# @(#)78	1.4  src/bos/diag/util/ulan/Makefile, dsalan, bos411, 9428A410j 12/15/93 11:04:09
#
#   COMPONENT_NAME: DSALAN
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1993.
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


PROGRAMS	= ulan

MSGHDRS		= ulan_msg.h
CATFILES 	= ulan.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc

ILIST		= ${PROGRAMS} ulan.cat
IDIR		= /usr/lpp/diagnostics/bin/
ulan.cat_IDIR = /usr/lpp/diagnostics/catalog/default/
ulan_OFILES	= arp.o device.o display.o do_icmp.o do_ip.o icmp.o \
		  parse_echo_reply.o parse_mac_frame.o parse_frame.o \
		  rarp.o read.o ring_diag.o saring.o start_device.o

.include <${RULES_MK}>
