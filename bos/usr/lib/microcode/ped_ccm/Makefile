# @(#)08	1.2  src/bos/usr/lib/microcode/ped_ccm/Makefile, dispccm, bos411, 9428A410j 3/5/93 14:59:35
#
#   COMPONENT_NAME: SYSXDISPCCM
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992,1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


#------------------------------------------------------------------------------
# CCM NOTES:
# None of this microcode goes into an MES kit
# None of this microcode is shipped on a diskette with the hardware
# The only release path for this micrcode is on bosboot or other such AIX media
# Therefore, there is no LPP_PACK required
# Therefore, there is no reason to invoke Make.ucod
#------------------------------------------------------------------------------

ILIST		= 8ee3c.01.01

IDIR		= /usr/lib/microcode/

OTHERS=

OTHERS+=8ee3c.01.01
8ee3c.01.01:8ee3c.01.01.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>

