# @(#)99	1.2  src/bldenv/mri2sf/Makefile, tenplus, bos412, GOLDA411a 3/25/93 17:51:07
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



VPATH = ${MAKETOP}/tenplus/sf_progs:${MAKETOP}/tenplus/lib/obj
PROGRAMS = mri2sf 

OFILES	= mri2sf.o sf_open.o sf_close.o sf_read.o sf_write.o s_free.o \
					s_newname.o s_realloc.o sf_seek.o fbackup.o sf_cookie.o \
					getl.o g_dtype.o fatal.o s_read.o s_alloc.o sf_stindx.o \
					s_write.o scopy.o glb_defs.o fexists.o puti.o putl.o geti.o \
					e_free.o c_print.o c_read.o c_write.o e_print.o e_read.o  \
					p_free.o p_print.o p_read.o p_write.o r_print.o noop.o r_read.o \
					r_write.o sf_free.o  sf_print.o e_write.o pindent.o s_print.o \
					s_append.o seq.o s_string.o

INCFLAGS = -I${MAKETOP}/tenplus/include

ILIST	= mri2sf
IDIR	= /usr/bin/

.include <${RULES_MK}>
