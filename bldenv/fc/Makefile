# @(#)97	1.2  src/bldenv/fc/Makefile, tenplus, bos412, GOLDA411a 3/25/93 17:46:58
#
# COMPONENT_NAME: (INED) INed Editor
# 
# ORIGINS:  27
# 
# This module contains IBM CONFIDENTIAL code. -- (IBM
# Confidential Restricted when combined with the aggregated
# modules for this product)                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1985, 1993
# All Rights Reserved
# 
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

VPATH = ${MAKETOP}/tenplus/tools/fc:${MAKETOP}/tenplus/lib/window:${MAKETOP}/tenplus/lib/util:${MAKETOP}/tenplus/lib/short:${MAKETOP}/tenplus/lib/obj
PROGRAMS = fc
fc_OFILES = fc.o common.o box.o \
					wn_init.o wn_write.o wn_free.o wn_print.o _wnprint.o wn_read.o  \
					a2string.o dispwidth.o attr_util.o packup.o unpackup.o smear.o  \
					i_delete.o i_insert.o i_smear.o i_realloc.o i_string.o i_strlen.o   \
					i_print.o i_read.o i_write.o  \
					i_addtype.o s_string.o s_print.o s_free.o fatal.o sf_close.o seq.o \
					sf_read.o s_findnode.o s_newname.o s_alloc.o s_append.o s_pointer.o \
					s_insert.o smear.o s_realloc.o sf_open.o fexists.o s_delete.o \
					sf_write.o nputc.o s_addtype.o noop.o debug.o glb_defs.o \
					s_settype.o  e_free.o g_dtype.o s_write.o sf_cookie.o \
					s_read.o sf_seek.o scopy.o getl.o sf_stindx.o fbackup.o pindent.o \
					geti.o puti.o c_print.o c_read.o c_write.o e_print.o e_read.o \
					e_write.o p_free.o p_print.o p_read.o p_write.o r_print.o r_read.o \
					r_write.o sf_free.o sf_print.o putl.o 

INCFLAGS = -I${MAKETOP}/tenplus/include

ILIST	= fc
IDIR	= /usr/bin/

.include <${RULES_MK}>
