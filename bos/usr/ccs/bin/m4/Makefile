# @(#)52        1.2  src/bos/usr/ccs/bin/m4/Makefile, cmdm4, bos412, 9445C412a 11/9/94 15:16:30
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 10,27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

CFLAGS			= -DXASLINE
PROGRAMS		= m4
MSGHDRS                 = m4_msg.h
CATFILES                = m4.cat

ILIST			= m4
IDIR			= /usr/ccs/bin/

OFILES			= m4.o m4y.o m4ext.o m4macs.o

.include <${RULES_MK}>

m4.o m4ext.o m4macs.o: m4.h m4_msg.h
