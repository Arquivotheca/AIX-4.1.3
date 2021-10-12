# @(#)98	1.5  src/bos/usr/lib/nls/loc/aix31/Makefile, cfgnlskm, bos411, 9428A410j 2/10/93 18:55:03
#
#   COMPONENT_NAME: CFGNLSKM
#
#   FUNCTIONS:		Makefile for imkeymaps
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

OTHERS = \
	Da_DK@aix31.imkeymap \
	De_CH@aix31.imkeymap \
	De_DE@aix31.imkeymap \
	En_GB@aix31.imkeymap \
	En_JP@aix31.imkeymap \
	En_US@aix31.imkeymap \
	Es_ES@aix31.imkeymap \
	Fr_BE@aix31.imkeymap \
	Fr_CA@aix31.imkeymap \
	Fr_CH@aix31.imkeymap \
	Fr_FR@aix31.imkeymap \
	Is_IS@aix31.imkeymap \
	It_IT@aix31.imkeymap \
	Ja_JP@aix31.imkeymap \
	Nl_BE@aix31.imkeymap \
	Nl_NL@aix31.imkeymap \
	No_NO@aix31.imkeymap \
	Pt_PT@aix31.imkeymap \
	Sv_SE@aix31.imkeymap \
	Fi_FI@aix31.imkeymap \
	En_GB@alt31.imkeymap \
	Fr_FR@alt31.imkeymap \
	It_IT@alt31.imkeymap \
	Sv_SE@alt31.imkeymap \
	Fi_FI@alt31.imkeymap

IDIR = /usr/lib/nls/loc/
ILIST = $(OTHERS)

.include <${RULES_MK}>


Da_DK@aix31.imkeymap: imkeymap.de
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

De_CH@aix31.imkeymap: imkeymap.sg
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

De_DE@aix31.imkeymap: imkeymap.gr
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

En_GB@aix31.imkeymap: imkeymap.uk
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

En_JP@aix31.imkeymap: imkeymap.ja
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

En_US@aix31.imkeymap: imkeymap.us
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

Es_ES@aix31.imkeymap: imkeymap.sp
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

Fr_BE@aix31.imkeymap: imkeymap.bl
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

Fr_CA@aix31.imkeymap: imkeymap.cf
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

Fr_CH@aix31.imkeymap: imkeymap.sf
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

Fr_FR@aix31.imkeymap: imkeymap.fr
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

Is_IS@aix31.imkeymap: imkeymap.is
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

It_IT@aix31.imkeymap: imkeymap.it
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

Ja_JP@aix31.imkeymap: imkeymap.jh
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

Nl_BE@aix31.imkeymap: imkeymap.be
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

Nl_NL@aix31.imkeymap: imkeymap.nl
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

No_NO@aix31.imkeymap: imkeymap.no
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

Pt_PT@aix31.imkeymap: imkeymap.po
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

Sv_SE@aix31.imkeymap: imkeymap.sw
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

Fi_FI@aix31.imkeymap: Sv_SE@aix31.imkeymap
	${CP} ${.ALLSRC} ${.TARGET}

En_GB@alt31.imkeymap: oimkeymap.uk
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

Fr_FR@alt31.imkeymap: oimkeymap.fr
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

It_IT@alt31.imkeymap: oimkeymap.it
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

Sv_SE@alt31.imkeymap: oimkeymap.sw
	${KEYCOMP} < ${.ALLSRC} > ${.TARGET} || > ${.TARGET}
	[ -s ${.TARGET} ] || ${RM} -f ${.TARGET}

Fi_FI@alt31.imkeymap: Sv_SE@alt31.imkeymap
	${CP} ${.ALLSRC} ${.TARGET}
