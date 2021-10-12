# @(#)97        1.1  src/bos/kernext/c327/Makefile, Makefiles, bos411, 9428A410j 8/5/92 18:08:51
# COMPONENT_NAME:
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

.include <${MAKETOP}bos/kernext/Kernext.mk>

KERNEL_EXT	= c327dd
IDIR		= /usr/lib/drivers/
ILIST		= c327dd

LIBS		= ${KERNEXT_LIBS}

c327dd_ENTRYPOINT	= c327config

c327dd_OFILES =	c327cutnpn.o c327cutpn.o c327diagdd.o c327npn.o	\
             	c327pn.o dftnsCmd.o dftnsOffl.o dftnsSubr.o	\
		tcaclose.o tcaioctl.o tcaintr.o tcampx.o	\
		tcaopen.o tcaread.o tcaselect.o tcasubnpn.o	\
		tcasubpn.o tcawrite.o


.include <${RULES_MK}>
