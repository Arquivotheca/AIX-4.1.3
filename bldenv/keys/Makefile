# @(#)98	1.2  src/bldenv/keys/Makefile, tenplus, bos412, GOLDA411a 3/25/93 17:48:26
#
# COMPONENT_NAME: (INED) INed Editor
# 
# ORIGINS:  9, 10, 27
# 
# This module contains IBM CONFIDENTIAL code. -- (IBM
# Confidential Restricted when combined with the aggregated
# modules for this product)                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1985, 1993
# All Rights Reserved
# 
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.


VPATH = ${MAKETOP}/tenplus/keys:${MAKETOP}/tenplus/lib/obj
PROGRAMS	= keys 
OFILES = keys.o sf_open.o sf_close.o sf_read.o sf_write.o s_free.o \
				s_addtype.o s_alloc.o seq.o s_append.o s_insert.o s_freenode.o \
				noop.o getl.o s_read.o sf_stindx.o fbackup.o s_write.o  \
				sf_cookie.o sf_seek.o e_free.o g_dtype.o s_realloc.o fatal.o \
				scopy.o geti.o e_read.o fexists.o puti.o e_write.o putl.o \
				c_print.o c_read.o c_write.o e_print.o p_free.o p_print.o  \
				p_read.o p_write.o r_print.o r_read.o r_write.o sf_free.o sf_print.o \
				debug.o pindent.o s_print.o glb_defs.o

INCFLAGS = -I${MAKETOP}/tenplus/include

ILIST	= keys
IDIR	= /usr/bin/

.include <${RULES_MK}>
