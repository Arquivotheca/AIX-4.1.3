# @(#)81	1.5  src/bos/usr/lib/nls/loc/uconvTable/Makefile, cmdiconv, bos41J, 9509A_all 2/19/95 23:42:27
#
#   COMPONENT_NAME:	CMDICONV
#
#   FUNCTIONS: none
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

OTHERS	=			\
	IBM-037			\
	IBM-273			\
	IBM-277			\
	IBM-278			\
	IBM-280			\
	IBM-284			\
	IBM-285			\
	IBM-297			\
	IBM-437			\
	IBM-500			\
	IBM-836			\
	IBM-837			\
	IBM-850			\
	IBM-852			\
	IBM-856			\
	IBM-857			\
	IBM-860			\
	IBM-861			\
	IBM-863			\
	IBM-865			\
	IBM-869			\
	IBM-875			\
	IBM-930			\
	IBM-932			\
	IBM-933			\
	IBM-934			\
	IBM-935			\
	IBM-936			\
	IBM-937			\
	IBM-938			\
	IBM-939			\
	IBM-942			\
	IBM-944			\
	IBM-946			\
	IBM-948			\
	IBM-1026		\
	IBM-1046		\
	IBM-1381		\
	IBM-eucJP		\
	IBM-eucKR		\
	IBM-eucTW		\
	IBM-eucCN		\
	IBM-sbdTW		\
	IBM-udcJP		\
	IBM-udcTW		\
	ISO8859-1		\
	ISO8859-2		\
	ISO8859-3		\
	ISO8859-4		\
	ISO8859-5		\
	ISO8859-6		\
	ISO8859-7		\
	ISO8859-8		\
	ISO8859-9		\
	JISX0201.1976-0		\
	JISX0208.1983-0		\
	KSC5601.1987-0		\
	CNS11643.1986-1		\
	CNS11643.1986-1-GR	\
	CNS11643.1986-2		\
	CNS11643.1986-2-GR	\
	CNS11643.CT-GR		\
	GB2312.1980-0		\
	GB2312.1980-0-GR

ILIST	= ${OTHERS} ${OTHERS:=.ucmap} ucs.alias README
IDIR	= /usr/lib/nls/loc/uconvTable/

.include <${RULES_MK}>

${OTHERS}: $$@.ucmap
	${UCONVDEF} -f ${$@.ucmap:P} $@
