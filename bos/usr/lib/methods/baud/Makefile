# @(#)84        1.2.1.1  src/bos/usr/lib/methods/baud/Makefile, foxclub, bos41J, 9520A_a 4/28/95 14:25:13
#
# COMPONENT_NAME: (CFGMETHODS) Configuration Methods
#
# FUNCTIONS: none
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1993, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#-------------------------------------------------------------------------
#	Makefile for device methods
#-------------------------------------------------------------------------

VPATH = ${MAKETOP}../../../../diag/tu/baud


#-----------------------------------------------------------------------
#	Un-comment some or all of these for debugging
#-----------------------------------------------------------------------

# CFLAGSLOCAL= -g -DDEBUG -DDEBUG_BUS -DCFGDEBUG
# COPTFLAG=
# STRIP_FLAG=
# LIST=list


#------------------------------------------------------------------------
# PROGRAMS are the names of the methods to build via this Makefile
#------------------------------------------------------------------------

PROGRAMS = cfgdiagbaud ucfgdiagbaud


#---------------------------------------------------------------------
# one of these for each program
#---------------------------------------------------------------------

cfgdiagbaud_OFILES = cfgdiagbaud.o
ucfgdiagbaud_OFILES = ucfgdiagbaud.o

INCFLAGS = -I../../../../diag/tu/baud

ILIST = cfgdiagbaud ucfgdiagbaud
IDIR = /usr/lib/methods/

#LIBS	= $(LIBDIR)/libc.a \
#	$(LIBDIR)/libcfg.a \
#	$(LIBDIR)/libodm.a \
#	$(LIBDIR)/librts.a \
#	$(LIBDIR)/libs.a

LIBS = -ls -lodm -lcfg -lrts

NOUPDATE_LIST = /usr/lib/methods/cfgdiagbaud \
		/usr/lib/methods/ucfgdiagbaud

.include <${RULES_MK}>
