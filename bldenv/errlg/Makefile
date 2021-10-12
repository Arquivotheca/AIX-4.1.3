# @(#)01	1.11  src/bldenv/errlg/Makefile, cmderrlg, bos412, GOLDA411a 11/9/93 16:59:37
#
#   COMPONENT_NAME: BOSBUILD
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
VPATH           = ${MAKETOP}/bos/usr/bin/errlg:${MAKETOP}/bos/usr/bin/errlg/libras

EXPINC_SUBDIRS	= msg odm libras errinstall liberrlg

EXPLIB_SUBDIRS	= libras liberrlg

SUBDIRS		= errinstall errupdate errprefix

INCLUDES	= errlg.h codepoint.h libras.h errids.h
errids.h_EXPDIR = /usr/include/sys/
EXPDIR		= /usr/include/

.include <${RULES_MK}>
