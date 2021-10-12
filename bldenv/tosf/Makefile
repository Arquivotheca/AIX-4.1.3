# @(#)01	1.4  src/bldenv/tosf/Makefile, tenplus, bos412, GOLDA411a 1/12/94 17:34:13
#
# COMPONENT_NAME: (INED) INed Editor
# 
# ORIGINS:  9, 10, 27
# 
# This module contains IBM CONFIDENTIAL code. -- (IBM
# Confidential Restricted when combined with the aggregated
# modules for this product)                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1985, 1994
# All Rights Reserved
# 
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.


# The path to libc and the references to AFnxtrec.o and AFread.o are needed
# because tosf calls AFnxtrec() which calls AFread(), and AFread() has a
# fix in 4.1 to handle "\ddd" and "\n" correctly.  Once all builds are being
# done on 4.1 or later, then these references can be deleted.

VPATH		= ${MAKETOP}/tenplus/sf_progs:${MAKETOP}/tenplus/lib/obj:${MAKETOP}/bos/usr/ccs/lib/libc

PROGRAMS 	= tosf 

OFILES		= tosf.o af.o sf_open.o sf_close.o sf_read.o \
		  sf_write.o s_free.o s_newname.o s_realloc.o \
		  sf_seek.o fbackup.o sf_cookie.o  getl.o \
		  g_dtype.o fatal.o s_read.o s_alloc.o sf_stindx.o \
		  s_write.o scopy.o glb_defs.o fexists.o puti.o \
		  putl.o geti.o e_free.o c_print.o c_read.o \
		  c_write.o e_print.o e_read.o p_free.o p_print.o \
		  p_read.o p_write.o r_print.o noop.o r_read.o \
		  r_write.o sf_free.o  sf_print.o e_write.o \
		  pindent.o s_print.o AFnxtrec.o  AFread.o

INCFLAGS	= -I${MAKETOP}/tenplus/include

ILIST		= tosf
IDIR		= /usr/bin/

.include <${RULES_MK}>
