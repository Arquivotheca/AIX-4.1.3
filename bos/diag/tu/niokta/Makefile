# static char sccsid[] = "@(#)22  1.2  src/bos/diag/tu/niokta/Makefile, tu_adpkbd, bos411, 9431A411a 7/19/94 15:31:41";
#
# COMPONENT_NAME: tu_adpkbd
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# Makefile HTX/Mfg. Keyboard Adapter Test Unit library
# 
################################################################################

# Start DBX support lines
#OPT_LEVEL       = -g
#CFLAGS          = -D_BSD -qdbxextra
#STRIP_FLAG      =
# End DBX support lines

LIBRARIES       = libakbdtu.a

EXPLIB_TARGETS  = export_libakbdtu.a
EXPDIR		= /usr/lib/

LIBS            =

OFILES          = \
	          exectu.o \
                  kbd_io.o \
                  kbd_fns.o \
                  sem.o \
                  mktu_rc.o \
                  tu10.o \
                  tu20.o \
	          tu30.o \
                  tu40.o


# CFLAGS		=   -Dnodiag

ILIST           =
IDIR            =

.include <${RULES_MK}>

