# @(#)99	1.14  src/bos/usr/lib/nls/loc/km/Makefile, cfgnlskm, bos41J, 9520A_all 5/10/95 14:06:43
#
#   COMPONENT_NAME: CFGNLSKM
#
#   FUNCTIONS: Makefile for imkeymaps
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
OTHERS = \
	C.imkeymap \
	Da_DK.IBM-850.imkeymap \
	De_BE.IBM-850.imkeymap \
	De_CH.IBM-850.imkeymap \
	De_DE.IBM-850.imkeymap \
	En_GB.IBM-850.imkeymap \
	En_US.IBM-850.imkeymap \
	Es_ES.IBM-850.imkeymap \
	Fi_FI.IBM-850.imkeymap \
	Fr_BE.IBM-850.imkeymap \
	Fr_CA.IBM-850.imkeymap \
	Fr_CH.IBM-850.imkeymap \
	Fr_FR.IBM-850.imkeymap \
	Is_IS.IBM-850.imkeymap \
	It_IT.IBM-850.imkeymap \
	Ja_JP.IBM-932.imkeymap \
	Nl_BE.IBM-850.imkeymap \
	Nl_NL.IBM-850.imkeymap \
	No_NO.IBM-850.imkeymap \
	Pt_PT.IBM-850.imkeymap \
	Sv_SE.IBM-850.imkeymap \
	da_DK.ISO8859-1.imkeymap \
	de_BE.ISO8859-1.imkeymap \
	de_CH.ISO8859-1.imkeymap \
	de_DE.ISO8859-1.imkeymap \
	en_GB.ISO8859-1.imkeymap \
	en_US.ISO8859-1.imkeymap \
	es_ES.ISO8859-1.imkeymap \
	fi_FI.ISO8859-1.imkeymap \
	fr_BE.ISO8859-1.imkeymap \
	fr_CA.ISO8859-1.imkeymap \
	fr_CH.ISO8859-1.imkeymap \
	fr_FR.ISO8859-1.imkeymap \
	is_IS.ISO8859-1.imkeymap \
	it_IT.ISO8859-1.imkeymap \
	ja_JP.IBM-eucJP.imkeymap \
	ko_KR.IBM-eucKR.imkeymap \
	nl_BE.ISO8859-1.imkeymap \
	nl_NL.ISO8859-1.imkeymap \
	no_NO.ISO8859-1.imkeymap \
	pt_BR.ISO8859-1.imkeymap \
	pt_PT.ISO8859-1.imkeymap \
	sv_SE.ISO8859-1.imkeymap \
	zh_CN.IBM-eucCN.imkeymap \
	ZH_CN.UTF-8.imkeymap \
	zh_TW.IBM-eucTW.imkeymap \
	zh_TW@alt.imkeymap \
	En_GB.IBM-850@alt.imkeymap \
	Fi_FI.IBM-850@alt.imkeymap \
	Fr_FR.IBM-850@alt.imkeymap \
	It_IT.IBM-850@alt.imkeymap \
	Sv_SE.IBM-850@alt.imkeymap \
	en_GB.ISO8859-1@alt.imkeymap \
	fi_FI.ISO8859-1@alt.imkeymap \
	fr_FR.ISO8859-1@alt.imkeymap \
	it_IT.ISO8859-1@alt.imkeymap \
	sv_SE.ISO8859-1@alt.imkeymap \
	cs_CZ.ISO8859-2.imkeymap \
	hr_HR.ISO8859-2.imkeymap \
	hu_HU.ISO8859-2.imkeymap \
	pl_PL.ISO8859-2.imkeymap \
	ro_RO.ISO8859-2.imkeymap \
	sh_SP.ISO8859-2.imkeymap \
	sl_SI.ISO8859-2.imkeymap \
	sk_SK.ISO8859-2.imkeymap \
	bg_BG.ISO8859-5.imkeymap \
	mk_MK.ISO8859-5.imkeymap \
	ru_RU.ISO8859-5.imkeymap \
	sr_SP.ISO8859-5.imkeymap \
	el_GR.ISO8859-7.imkeymap \
	tr_TR.ISO8859-9.imkeymap \
	Ar_AA.IBM-1046.imkeymap \
	Ar_AA.IBM-1046@alt.imkeymap \
	Iw_IL.IBM-856.imkeymap \
	Iw_IL.IBM-856@alt.imkeymap \
	ar_AA.ISO8859-6.imkeymap \
	ar_AA.ISO8859-6@alt.imkeymap \
	iw_IL.ISO8859-8.imkeymap \
	iw_IL.ISO8859-8@alt.imkeymap \
	C@outbound.imkeymap \
	Ja_JP@aixterm.imkeymap \
	ja_JP@aixterm.imkeymap

IDIR = /usr/lib/nls/loc/
ILIST = $(OTHERS) $(OTHERS:=.src) dead.imkeymap.src

.include <${RULES_MK}>

${OTHERS}:	$${.TARGET:=.src}
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

