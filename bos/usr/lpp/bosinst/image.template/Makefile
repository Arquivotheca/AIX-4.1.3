# @(#) 42 1.3 src/bos/usr/lpp/bosinst/image.template/Makefile, bosinst, bos411, 9428A410j 7/23/93 14:24:26
# COMPONENT_NAME: bosinst
#
# FUNCTIONS: Makefile for image.template
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

ILIST			= image.template image.data
image.template_IDIR	= /usr/lpp/bosinst/
image.data_IDIR		= /

.include <${RULES_MK}>

image.data: image.template
	${CP} ${image.template:P} image.data

