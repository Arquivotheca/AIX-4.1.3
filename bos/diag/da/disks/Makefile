# @(#)18	1.3  src/bos/diag/da/disks/Makefile, dadisks, bos411, 9428A410j 12/5/93 20:50:39
#
#   COMPONENT_NAME: DADISKS
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


VPATH		= ${MAKETOP}/bos/diag/tu/scsi:${MAKETOP}/bos/diag/util/uformat:${MAKETOP}/bos/diag/tu/bad
PROGRAMS	= dhf dhfd
SCRIPTS		= dgvars

MSGHDRS		= dhf_msg.h dhflvm_msg.h dhfd_msg.h
CATFILES 	= dhf.cat dhflvm.cat dhfd.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc

ILIST		= ${PROGRAMS} dhf.cat dgvars dhfd.cat dhflvm.cat
IDIR		= /usr/lpp/diagnostics/da/
dhf.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/
dhfd.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/
dhflvm.cat_IDIR = /usr/lpp/diagnostics/catalog/default/

dhf_OFILES	= dhf_mn.o  dhflvm.o dhf.o scsi_atu.o disktools.o

dhfd_OFILES	= dhfd_mn.o dhflvm.o dhfd.o badatu.o
.include <${RULES_MK}>
