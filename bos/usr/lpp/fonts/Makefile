# @(#)80        1.7  src/bos/usr/lpp/fonts/Makefile, Makefiles, bos411, 9428A410j 2/18/94 10:08:28
# COMPONENT_NAME: cfgnls
#
# FUNCTIONS:
#
# ORIGINS: 27
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

OTHERS = \
	Rom10.iso1.snf \
	Rom10.iso2.snf \
	Rom10.iso5.snf \
	Rom10.iso7.snf \
	Rom10.iso8.snf \
	Rom10.iso9.snf \
	Rom10.ibm1046.snf \
	Rom10.ibm850.snf \
	Rom10.ibm856.snf \
	Rom22.iso1.snf \
	Rom22.iso2.snf \
	Rom22.iso5.snf \
	Rom22.iso7.snf \
	Rom22.iso8.snf \
	Rom22.iso9.snf \
	Rom22.ibm1046.snf \
        Rom22.ibm850.snf \
	Rom22.ibm856.snf \
	Erg11.iso1.snf \
	Erg11.iso7.snf \
	Erg11.iso9.snf \
	Erg11.ibm850.snf \
	Erg22.iso1.snf \
	Erg22.iso7.snf \
	Erg22.iso9.snf \
	Erg22.ibm850.snf

ILIST 	= ${OTHERS}
IDIR	= /usr/lpp/fonts/

.include <${RULES_MK}>

.SUFFIXES: .snf .bdf

.bdf.snf:
	${BDFTOSNF} -i ${.IMPSRC} > ${.TARGET}
