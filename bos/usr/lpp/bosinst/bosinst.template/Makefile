# @(#) 40 1.3 src/bos/usr/lpp/bosinst/bosinst.template/Makefile, bosinst, bos411, 9428A410j 7/23/93 14:25:55
# COMPONENT_NAME: bosinst
#
# FUNCTIONS: Makefile for bosinst.template
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

ILIST			= bosinst.template bosinst.data
bosinst.template_IDIR	= /usr/lpp/bosinst/
bosinst.data_IDIR	= /

.include <${RULES_MK}>

bosinst.data: bosinst.template
	${CP} ${bosinst.template:P} bosinst.data

