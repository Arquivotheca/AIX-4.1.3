# @(#)88	1.1  src/bos/kernext/scsi/wordrev.s, sysxscsi, bos411, 9428A410j 6/26/91 11:07:06
#*****************************************************************************
# COMPONENT_NAME: (SYSXDISK)
#
# FUNCTIONS: word_reverse
#
# ORIGINS: 
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1990
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

        .file "wordrev.s"

#-----------------------------------------------------------------------------
#
# NAME: word_reverse
#
# FUNCTION: This routine returns the byte reverse word of data. 
#
# CALL: (int)word_reverse(word)
#       uint word;            /* 32-bit word             */
#
# EXECUTION ENVIORNMENT:
#       This service can be called on either the process or
#       interrupt level
#
#       It only page faults on the stack when under a process
#
# NOTES:
#       This is a specialized interface for speeding up register handling 
#		for the salmon scsi driver.
#
# RETURN VALUE:
#
#-----------------------------------------------------------------------------
 	S_PROLOG(word_reverse)
	st    r3,-4(r1)		# store at end of the stack
	liu   r12,0x0
	lil   r12,-4
	lbrx  r3,r12,r1		# read back in byte reversed
 	S_EPILOG(word_reverse)
