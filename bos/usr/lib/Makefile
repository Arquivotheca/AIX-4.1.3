# @(#)35	1.31  src/bos/usr/lib/Makefile, Makefiles, bos41J, 9507B 1/26/95 09:59:20
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

EXPINC_SUBDIRS		= nim pios piosk pioskr piosCN methods nls netls ncs \
			  rcm \
					  microcode

EXPLIB_SUBDIRS		= nim methods piosk piosCN netls ncs

SUBDIRS		= \
	asw bugfiler bugformat nim learn methods ncs netls nls objrepos \
	pios piosk pioskr piostw piosCN sendmail unixtomh boot lpd security sa \
	rcm \
	assist microcode ${${TARGET_MACHINE}_SUBDIRS}

ILIST		= dsinfo
IDIR		= /usr/lib/

.include <${RULES_MK}>
