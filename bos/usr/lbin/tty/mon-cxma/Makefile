# @(#)04	1.1  src/bos/usr/lbin/tty/mon-cxma/Makefile, sysxtty, bos411, 9428A410j 6/23/94 15:46:54
#
# COMPONENT_NAME: (sysxtty) System Extension for tty support
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1994
# All Rights Reserved
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Licensed Material - Property of IBM
#

PROGRAMS                = mon-cxma

IDIR                    = /usr/lbin/tty/
ILIST                   = mon-cxma

CCTYPE                  = writable_strings
OFILES                  = mon-cxma.o vpd.o term.o loop_back.o tables.o status.o

LIBS                    = -lcfg -lodm -lxcurses

.include <${RULES_MK}>
