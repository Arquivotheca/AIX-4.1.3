# @(#)13        1.1  src/bos/usr/lib/methods/cfgdgric/Makefile, dd_artic, bos411, 9428A410j 11/11/93 14:59:48
#
# COMPONENT_NAME: dd_artic -- ARTIC Diagnostic Device driver.
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1991, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#############################################################################

#-----------------------------------------------------------------------#
#                                                                       #
#       This makefile makes driver cfg method for artic                 #
#                                                                       #
#-----------------------------------------------------------------------#

VPATH		= ${MAKETOP}/bos/usr/lib/methods/common

PROGRAMS	= cfgdgric

OFILES		= cfgdgric.o

LIBS      	= -lcfg -lodm -lrts -ls

INCFLAGS	= -I${MAKETOP}/bos/usr/lib/methods/common

ILIST      = cfgdgric
IDIR       = /usr/lib/methods/

.include <${RULES_MK}>
