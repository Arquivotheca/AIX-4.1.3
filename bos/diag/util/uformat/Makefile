# @(#)15	1.5  src/bos/diag/util/uformat/Makefile, dsauformat, bos41J, 9512A_all 3/16/95 17:48:13
#
#   COMPONENT_NAME: DSAUFORMAT
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


VPATH		= ${MAKETOP}/bos/diag/util/uformat/serialdisk:${MAKETOP}/bos/diag/tu/scsi
PROGRAMS	= uformat kazfor

SUBDIRS		= serialdisk
MSGHDRS		= ufmt_msg.h kazfor_msg.h
CATFILES 	= ufmt.cat kazfor.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc

ILIST		= ${PROGRAMS} ${CATFILES}
IDIR		= /usr/lpp/diagnostics/bin/
ufmt.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/
kazfor.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

OBJECTS		= disktools.o
uformat_OFILES	= disktools.o  uformat.o scsi_atu.o disp_mn.o \
		  udclass.o bad.o disp_patt.o \
		  dt.o
         	
kazfor_OFILES	= kazfor.o disp_mn.o

.include <${RULES_MK}>
