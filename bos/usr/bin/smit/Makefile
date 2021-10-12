# @(#)28        1.14  src/bos/usr/bin/smit/Makefile, cmdsmit, bos41J, 9514A_all 4/4/95 13:23:45
# COMPONENT_NAME: cmdsmit (SMIT) System Management Interface Tool
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

CFLAGS=-D_ORI2 -D_SMIT41
EXPINC_SUBDIRS  = odm_msg
SUBDIRS		= odm_msg

SCRIPTS		= smit

PROGRAMS	= smitty

OFILES		=  smit_class.o \
	csmitcommand.o\
	csmitdialog.o\
	csmithelp.o\
	csmitlist.o\
	csmitmain.o\
	csmitmenu.o\
	csmitnamesel.o\
	csmitsignal.o\
	csmitutils.o \
	csmitxsystem.o
#	srex.o\
#	crex.o


LIBS		= -lori2 -lodm -lcfg -lasl -liconv -ls

INCFLAGS	= -Iodm_msg 

ILIST		= smit smitty smit_stanzas
IDIR		= /usr/bin/
smit_stanzas_IDIR = /usr/samples/smit/


.include <${RULES_MK}>

