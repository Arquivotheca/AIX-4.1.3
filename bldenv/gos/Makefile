# @(#)93        1.11  src/bldenv/gos/Makefile, x11, bos412, GOLDA411a 8/18/94 16:21:45
#
#   COMPONENT_NAME: x11
#
#   FUNCTIONS:
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

EXPINC_SUBDIRS	= mkfontdir libfont

EXPLIB_SUBDIRS	= libfont
.if exists(${MAKETOP}/cde/)
EXPLIB_SUBDIRS	+= desktop
.endif

SUBDIRS		= mkfontdir pswrap custom rgb Xlib
.if exists(${MAKETOP}/cde/)
SUBDIRS		+= desktop
.endif
SUBDIRS		+= xcpp
.if exists(${MAKETOP}/gos/xdt2cde/)
SUBDIRS		+= xdt2cde
.endif

.include <${RULES_MK}>


