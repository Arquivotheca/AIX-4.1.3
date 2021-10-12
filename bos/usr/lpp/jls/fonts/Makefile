# @(#)89        1.7  src/bos/usr/lpp/jls/fonts/Makefile, fontKJI, bos411, 9428A410j 3/24/94 08:41:45
#
# COMPONENT_NAME: fontKJI
#
# FUNCTIONS: Japanese Default Fonts
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

SUBDIRS	= \
	misc

FONT_LIST = \
	RomanKn12.pcf.Z  \
        RomanKn17.pcf.Z  \
        RomanKn23.pcf.Z  \
        RomanKn23G.pcf.Z \
	Kana12.pcf.Z     \
        Kana17.pcf.Z     \
        Kana23.pcf.Z     \
        Kana23G.pcf.Z    \
	Kanji12.pcf.Z    \
        Kanji17.pcf.Z    \
        Kanji23.pcf.Z    \
        Kanji23G.pcf.Z   \
	IBM_JPN12.pcf.Z  \
        IBM_JPN17.pcf.Z  \
        IBM_JPN23.pcf.Z  \
        IBM_JPN23G.pcf.Z

SCRIPTS	= fonts.alias.JP

OTHERS	= ${FONT_LIST}

ILIST 	= ${OTHERS} ${SCRIPTS}
IDIR	= /usr/lpp/X11/lib/X11/fonts/

.include <${RULES_MK}>

${FONT_LIST:M*.pcf.Z} : $${.TARGET:.pcf.Z=.bdf}
	${RM} -f ${.TARGET} ${.TARGET:.Z=}
	${BDFTOPCF} ${.ALLSRC} >${.TARGET:.Z=}
	${COMPRESS} ${.TARGET:.Z=}
