# @(#)58        1.6  src/bos/usr/bin/errlg/en_US/Makefile, cmderrlg, bos411, 9436D411a 9/8/94 14:20:27
# COMPONENT_NAME: CMDERRLG
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

OTHERS		= codepoint.cat bos.sysmgt.serv_aid.codept

ILIST		= ${OTHERS} 
IDIR		= /usr/lib/nls/msg/${MSGLANG}/
bos.sysmgt.serv_aid.codept_IDIR		= /usr/lpp/bos.sysmgt/serv_aid/

CODEPOINT_LIST	= \
	codepoint_setD.mri \
	codepoint_setE.mri \
	codepoint_setF.mri \
	codepoint_setI.mri \
	codepoint_setP.mri \
	codepoint_setR.mri \
	codepoint_setU.mri
   
.include <${RULES_MK}>

codepoint.mri:	${CODEPOINT_LIST}
	${RM} -f ${.TARGET}
	${CAT} ${.ALLSRC} > ${.TARGET}

bos.sysmgt.serv_aid.codept:	codepoint.mri
	${RM} -f ${.TARGET}
	${ERRPREFIX} ${codepoint.mri:P} - > ${.TARGET}

codepoint.cat:	bos.sysmgt.serv_aid.codept
	${RM} -f ${.TARGET}
	${ERRINST} -q -z ${.TARGET} bos.sysmgt.serv_aid.codept
