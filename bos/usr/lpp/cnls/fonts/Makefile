# @(#)89	1.4  src/bos/usr/lpp/cnls/fonts/Makefile, ils-zh_CN, bos41J, 9513A_all 3/23/95 04:03:19
#
#   COMPONENT_NAME: ils-zh_CN
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
FONT_LIST = \
	12x24cn.pcf.Z \
	24x48cn.pcf.Z \
	26x54rom.pcf.Z \
	52x54gb.pcf.Z \
	cf48.pcf.Z \
	ch48.pcf.Z \
	ck48.pcf.Z \
	clib24f.pcf.Z \
	clib24h.pcf.Z \
	clib24k.pcf.Z \
	clib24s.pcf.Z \
	cs48.pcf.Z \
	ibm27.pcf.Z \
	gb27s.pcf.Z \
	rom27s.pcf.Z \
	udccn.pcf.Z \
	48udccn.pcf.Z \
	52x54sbdcn.pcf.Z

OTHERS	= ${FONT_LIST}

SCRIPTS = fonts.alias.CN

ILIST   = ${SCRIPTS} ${OTHERS}
IDIR    = /usr/lpp/X11/lib/X11/fonts/

.include <${RULES_MK}>

${FONT_LIST:M*.pcf.Z} : $${.TARGET:.pcf.Z=.bdf}
	${RM} -f ${.TARGET} ${.TARGET:.Z=}
	${BDFTOPCF} ${.ALLSRC} >${.TARGET:.Z=}
	${COMPRESS} ${.TARGET:.Z=}
