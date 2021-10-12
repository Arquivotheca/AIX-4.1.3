# @(#)98        1.3  src/bldenv/tdigest/Makefile, tenplus, bos412, GOLDA411a 5/12/93 18:24:26
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

VPATH = ${MAKETOP}/tenplus/util:${MAKETOP}/tenplus/lib/obj
PROGRAMS = tdigest

tdigest_OFILES = tdigest.o inseqs.o  sf_open.o sf_read.o fatal.o s_alloc.o \
                 sf_write.o s_free.o sf_close.o s_string.o s_newname.o \
                 s_findnode.o s_typchk.o s_realloc.o s_append.o \
                 s_inlist.o getl.o s_read.o sf_stindx.o fbackup.o \
                 sf_seek.o scopy.o s_write.o e_free.o  sf_cookie.o \
                 glb_defs.o g_dtype.o e_read.o e_write.o putl.o puti.o \
                 c_print.o c_read.o c_write.o e_print.o p_free.o p_print.o \
                 p_read.o p_write.o r_print.o noop.o r_read.o r_write.o \
                 sf_free.o sf_print.o fexists.o pindent.o s_print.o \
                 geti.o



CATFILES = tdigest.cat
MSGHDRS = tdigest_msg.h
HFILES = ${MSGHDRS}

INCFLAGS = -I${MAKETOP}/tenplus/include -I${MAKETOP}/tenplus/util

ILIST	= tdigest
IDIR	= /usr/bin/

.include <${RULES_MK}>
