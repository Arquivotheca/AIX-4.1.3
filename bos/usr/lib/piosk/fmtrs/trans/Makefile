# @(#)77        1.4  src/bos/usr/lib/piosk/fmtrs/trans/Makefile, cmdpiosk, bos41J, 9522A_all 5/30/95 11:07:00
#
#   COMPONENT_NAME: CMDPIOSK
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

DATAFILES	= sjis83_IBM-932 IBM-932_sjis83 \
		  sjis90_IBM-932 IBM-932_sjis90 \
		  euc78_IBM-eucJP IBM-eucJP_euc78 \
		  euc83_IBM-eucJP IBM-eucJP_euc83 \
		  jis78_fold7 fold7_jis78 \
		  jis83_fold7 fold7_jis83 \
		  jis90_fold7 fold7_jis90

ILIST		= ${DATAFILES} codeset.alias trans.dir
IDIR		= /usr/lib/lpd/pio/transJP/

install_sjis83_IBM-932: IBM-932_sjis83
install_sjis90_IBM-932: IBM-932_sjis90
install_euc78_IBM-eucJP: IBM-eucJP_euc78
install_euc83_IBM-eucJP: IBM-eucJP_euc83
install_jis78_fold7: fold7_jis78
install_jis83_fold7: fold7_jis83
install_jis90_fold7: fold7_jis90

.include <${RULES_MK}>

sjis83_IBM-932:
	@${RM} -f ${.TARGET}
	${TOUCH} ${.TARGET}

IBM-932_sjis83 : ${S83_932} sjis83_IBM-932
	@${RM} -f ${.TARGET} sjis83_IBM-932
	${S83_932}
	${TOUCH} ${.TARGET}

sjis90_IBM-932:
	@${RM} -f ${.TARGET}
	${TOUCH} ${.TARGET}

IBM-932_sjis90 : ${S90_932} sjis90_IBM-932
	@${RM} -f ${.TARGET} sjis90_IBM-932
	${S90_932}
	${TOUCH} ${.TARGET}

euc78_IBM-eucJP:
	@${RM} -f ${.TARGET}
	${TOUCH} ${.TARGET}

IBM-eucJP_euc78 : ${E78_EUCJP} euc78_IBM-eucJP
	@${RM} -f ${.TARGET} euc78_IBM-eucJP
	${E78_EUCJP}
	${TOUCH} ${.TARGET}

euc83_IBM-eucJP:
	@${RM} -f ${.TARGET}
	${TOUCH} ${.TARGET}

IBM-eucJP_euc83 : ${E83_EUCJP} euc83_IBM-eucJP
	@${RM} -f ${.TARGET} euc83_IBM-eucJP
	${E83_EUCJP}
	${TOUCH} ${.TARGET}

jis78_fold7:
	@${RM} -f ${.TARGET}
	${TOUCH} ${.TARGET}

fold7_jis78 : ${J78_FOLD7} jis78_fold7
	@${RM} -f ${.TARGET} jis78_fold7
	${J78_FOLD7}
	${TOUCH} ${.TARGET}

jis83_fold7:
	@${RM} -f ${.TARGET}
	${TOUCH} ${.TARGET}

fold7_jis83 : ${J83_FOLD7} jis83_fold7
	@${RM} -f ${.TARGET} jis83_fold7
	${J83_FOLD7}
	${TOUCH} ${.TARGET}

jis90_fold7:
	@${RM} -f ${.TARGET}
	${TOUCH} ${.TARGET}

fold7_jis90 : ${J90_FOLD7} jis90_fold7
	@${RM} -f ${.TARGET} jis90_fold7
	${J90_FOLD7}
	${TOUCH} ${.TARGET}
