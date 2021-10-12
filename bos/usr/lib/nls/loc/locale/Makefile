# @(#)25	1.25  src/bos/usr/lib/nls/loc/locale/Makefile, cfgnls, bos41J, 9512A_all 3/8/95 13:43:31
#
#   COMPONENT_NAME: CFGNLS
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993,1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

SUBDIRS = ko_KR zh_TW zh_CN ZH_CN

VPATH 		= ${MAKETOP}bos/usr/lib/nls/charmap

LOCALE_SRC	= ar_AA.ISO8859-6.src \
		  bg_BG.ISO8859-5.src \
		  ca_ES.ISO8859-1.src \
		  cs_CZ.ISO8859-2.src \
		  da_DK.ISO8859-1.src \
		  de_CH.ISO8859-1.src \
		  de_DE.ISO8859-1.src \
		  el_GR.ISO8859-7.src \
		  en_GB.ISO8859-1.src \
		  en_US.ISO8859-1.src \
		  es_ES.ISO8859-1.src \
		  fi_FI.ISO8859-1.src \
		  fr_BE.ISO8859-1.src \
		  fr_CA.ISO8859-1.src \
		  fr_CH.ISO8859-1.src \
		  fr_FR.ISO8859-1.src \
		  hr_HR.ISO8859-2.src \
		  hu_HU.ISO8859-2.src \
		  is_IS.ISO8859-1.src \
		  it_IT.ISO8859-1.src \
		  iw_IL.ISO8859-8.src \
		  ja_JP.IBM-eucJP.src \
		  mk_MK.ISO8859-5.src \
		  nl_BE.ISO8859-1.src \
		  nl_NL.ISO8859-1.src \
		  no_NO.ISO8859-1.src \
		  pl_PL.ISO8859-2.src \
		  pt_BR.ISO8859-1.src \
		  pt_PT.ISO8859-1.src \
		  ro_RO.ISO8859-2.src \
		  ru_RU.ISO8859-5.src \
		  sh_SP.ISO8859-2.src \
		  sl_SI.ISO8859-2.src \
		  sk_SK.ISO8859-2.src \
		  sr_SP.ISO8859-5.src \
		  sv_SE.ISO8859-1.src \
		  tr_TR.ISO8859-9.src \
		  Ar_AA.IBM-1046.src \
		  Iw_IL.IBM-856.src

BUILT_LOCALE_SRC = Ca_ES.IBM-850.src \
		   Da_DK.IBM-850.src \
		   De_CH.IBM-850.src \
		   De_DE.IBM-850.src \
		   En_GB.IBM-850.src \
		   En_US.IBM-850.src \
		   Es_ES.IBM-850.src \
		   Fi_FI.IBM-850.src \
		   Fr_BE.IBM-850.src \
		   Fr_CA.IBM-850.src \
		   Fr_CH.IBM-850.src \
		   Fr_FR.IBM-850.src \
		   Is_IS.IBM-850.src \
		   It_IT.IBM-850.src \
		   Ja_JP.IBM-932.src \
		   Nl_BE.IBM-850.src \
		   Nl_NL.IBM-850.src \
		   No_NO.IBM-850.src \
		   Pt_PT.IBM-850.src \
		   Sv_SE.IBM-850.src

ALL_LOCALE_SRC	= ${LOCALE_SRC} ${BUILT_LOCALE_SRC}

LOCALES		= ${ALL_LOCALE_SRC:R}
OTHERS		= ${BUILT_LOCALE_SRC} ${LOCALES}

IDIR		= /usr/lib/nls/loc/
ILIST		= ${ALL_LOCALE_SRC} ${LOCALES}

.include <${RULES_MK}>

#
# Build	rules for locales.
#

# 
# These are various variables that we need.
#

BASEFLAGS	=-v -c -C '${_CCFLAGS_} -F${ODE_TOOLS}/etc/xlC.cfg' \
		 -P ${ODE_TOOLS}/usr
LDBINDFLAGS	=${LIBDIRS} -bbinder:${ODE_TOOLS}/usr/lib/bind \
		 -bglink:${ODE_TOOLS}/usr/lib/glink.o

NOMETH_PATH	=-blibpath:/usr/lib:/lib

LOCDEFFLAGS	=${BASEFLAGS} -L '-lc ${LDBINDFLAGS} ${NOMETH_PATH}'

#
# This is the rule that actually builds all the locales.
#

${LOCALES} : $${.TARGET:E} $${.TARGET:=.src}
	${LOCDEF} -f ${.ALLSRC:N*.src} -i ${.ALLSRC:M*.src} ${LOCDEFFLAGS} ${.TARGET} || [ $$? -lt 2 ]

#
# These rules are used to copy the ISO source file name to the IBM-850
# filename, so we build the locale for both codesets with the same 
# source file.

Ca_ES.IBM-850.src : ${ca_ES.ISO8859-1.src:P}
	${CP} ${.ALLSRC} ${.TARGET}

Da_DK.IBM-850.src : ${da_DK.ISO8859-1.src:P}
	${CP} ${.ALLSRC} ${.TARGET}

De_CH.IBM-850.src : ${de_CH.ISO8859-1.src:P}
	${CP} ${.ALLSRC} ${.TARGET}

De_DE.IBM-850.src : ${de_DE.ISO8859-1.src:P}
	${CP} ${.ALLSRC} ${.TARGET}

En_GB.IBM-850.src : ${en_GB.ISO8859-1.src:P}
	${CP} ${.ALLSRC} ${.TARGET}

En_US.IBM-850.src : ${en_US.ISO8859-1.src:P}
	${CP} ${.ALLSRC} ${.TARGET}

Es_ES.IBM-850.src : ${es_ES.ISO8859-1.src:P}
	${CP} ${.ALLSRC} ${.TARGET}

Fi_FI.IBM-850.src : ${fi_FI.ISO8859-1.src:P}
	${CP} ${.ALLSRC} ${.TARGET}

Fr_BE.IBM-850.src : ${fr_BE.ISO8859-1.src:P}
	${CP} ${.ALLSRC} ${.TARGET}

Fr_CA.IBM-850.src : ${fr_CA.ISO8859-1.src:P}
	${CP} ${.ALLSRC} ${.TARGET}

Fr_CH.IBM-850.src : ${fr_CH.ISO8859-1.src:P}
	${CP} ${.ALLSRC} ${.TARGET}

Fr_FR.IBM-850.src : ${fr_FR.ISO8859-1.src:P}
	${CP} ${.ALLSRC} ${.TARGET}

Is_IS.IBM-850.src : ${is_IS.ISO8859-1.src:P}
	${CP} ${.ALLSRC} ${.TARGET}

It_IT.IBM-850.src : ${it_IT.ISO8859-1.src:P}
	${CP} ${.ALLSRC} ${.TARGET}

Ja_JP.IBM-932.src : ${ja_JP.IBM-eucJP.src:P}
	${CP} ${.ALLSRC} ${.TARGET}

Nl_BE.IBM-850.src : ${nl_BE.ISO8859-1.src:P}
	${CP} ${.ALLSRC} ${.TARGET}

Nl_NL.IBM-850.src : ${nl_NL.ISO8859-1.src:P}
	${CP} ${.ALLSRC} ${.TARGET}

No_NO.IBM-850.src : ${no_NO.ISO8859-1.src:P}
	${CP} ${.ALLSRC} ${.TARGET}

Pt_PT.IBM-850.src : ${pt_PT.ISO8859-1.src:P}
	${CP} ${.ALLSRC} ${.TARGET}

Sv_SE.IBM-850.src : ${sv_SE.ISO8859-1.src:P}
	${CP} ${.ALLSRC} ${.TARGET}
