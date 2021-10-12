# @(#)77       1.4  src/bldenv/locales/41/Makefile, ade_build, bos412, GOLDA411a 6/29/94 09:25:46
#
#   COMPONENT_NAME: ade_build
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993,1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#   This makefile will ship the /usr/lib/nls/loc/locales into the 
#   build environment under <build>/ode_tools/power/usr/lib/nls/loc.
#

LOCALE_BIN	= ar_AA.ISO8859-6 \
		  bg_BG.ISO8859-5 \
		  cs_CZ.ISO8859-2 \
		  da_DK.ISO8859-1 \
		  de_CH.ISO8859-1 \
		  de_DE.ISO8859-1 \
		  el_GR.ISO8859-7 \
		  en_GB.ISO8859-1 \
		  en_US.ISO8859-1 \
		  es_ES.ISO8859-1 \
		  fi_FI.ISO8859-1 \
		  fr_BE.ISO8859-1 \
		  fr_CA.ISO8859-1 \
		  fr_CH.ISO8859-1 \
		  fr_FR.ISO8859-1 \
		  hr_HR.ISO8859-2 \
		  hu_HU.ISO8859-2 \
		  is_IS.ISO8859-1 \
		  it_IT.ISO8859-1 \
		  iw_IL.ISO8859-8 \
		  ja_JP.IBM-eucJP \
		  ko_KR.IBM-eucKR \
		  mk_MK.ISO8859-5 \
		  nl_BE.ISO8859-1 \
		  no_NO.ISO8859-1 \
		  pl_PL.ISO8859-2 \
		  pt_PT.ISO8859-1 \
		  ro_RO.ISO8859-2 \
		  ru_RU.ISO8859-5 \
		  sh_SP.ISO8859-2 \
		  sl_SI.ISO8859-2 \
		  sk_SK.ISO8859-2 \
		  sr_SP.ISO8859-5 \
		  sv_SE.ISO8859-1 \
		  tr_TR.ISO8859-9 \
		  zh_TW.IBM-eucTW

BUILT_LOCALE_BIN = Ar_AA.IBM-1046 \
		   Da_DK.IBM-850 \
		   De_CH.IBM-850 \
		   En_GB.IBM-850 \
		   En_US.IBM-850 \
		   Es_ES.IBM-850 \
		   Fi_FI.IBM-850 \
		   Fr_BE.IBM-850 \
		   Fr_CA.IBM-850 \
		   Fr_CH.IBM-850 \
		   Fr_FR.IBM-850 \
		   Is_IS.IBM-850 \
		   It_IT.IBM-850 \
		   Iw_IL.IBM-856 \
		   Ja_JP.IBM-932 \
		   Nl_BE.IBM-850 \
		   No_NO.IBM-850 \
		   Pt_PT.IBM-850 \
		   Sv_SE.IBM-850

ALL_LOCALE_BIN	= ${LOCALE_BIN} ${BUILT_LOCALE_BIN}

OBJECT_FILES	= ko_KR.o zh_TW.o
OTHERS		=  ${ALL_LOCALE_BIN:R}

NEW_LOC		= ${TOSTAGE}${BLDENV_LOCPATH}
IDIR		= ${BLDENV_LOCPATH}/
ILIST		= ${OBJECT_FILES} ${ALL_LOCALE_BIN}

.include <${RULES_MK}>

#
#  This stanza will set the links to the locales that have been
#  shipped to the build environment.  The link will consist of the 
#  locale name without the suffix.
#

${ALL_LOCALE_BIN:R} :
	[ -d ${NEW_LOC} ] || \
		(${MKDIR} -p ${NEW_LOC});
	@curr_dir=`${PWD_SHELL}`; \
	${TOUCH} loc.mk; \
	cd ${NEW_LOC}; \
	locale_name=${ALL_LOCALE_BIN:M${.TARGET}*}; \
	[ -z "$$locale_name" ] || \
		(${ECHO} /usr/bin/ln -f -s "$$locale_name" ${.TARGET}; \
		/usr/bin/ln -f -s "$$locale_name" ${.TARGET}; \
		);  \
	cd $$curr_dir;
