# @(#)66	1.6  src/bos/diag/da/sys/Makefile, dasys, bos412, 9446A412a 11/9/94 07:07:05
#
#   COMPONENT_NAME: DASYS
#
#   FUNCTIONS: 
#
#   ORIGINS: 27, 83
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
#   LEVEL 1, 5 Years Bull Confidential Information
#



PROGRAMS	= dsys dmptu

dmptu_CCTYPE	= base_threaded
MSGHDRS		= sys_msg.h
CATFILES 	= sys.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lm -lc

ILIST		= ${PROGRAMS} sys.cat
IDIR		= /usr/lpp/diagnostics/da/
sys.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/


dsys_OFILES	= dsys.o
dmptu_OFILES	= dmptu.o

.include <${RULES_MK}>
