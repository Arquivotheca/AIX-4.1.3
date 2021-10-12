#*****************************************************************************
# COMPONENT_NAME: (SYSXSCSI)
#
# FUNCTIONS: asc_cntlz
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

        .file "asc_cntlz.s"

#-----------------------------------------------------------------------------
#
# NAME: sd_getfree
#
# FUNCTION: This routine returns the index of the first free element of a list
#	    by counting the leading zeroes of the word
#
# CALL: (int)asc_cntlz(word)
#       uint word;            /* 32-bit word             */
#
# EXECUTION ENVIORNMENT:
#       This service can be called on either the process or
#       interrupt level
#
#       It only page faults on the stack when under a process
#
# NOTES:
#       This is a specialized interface for list management for the 
#	SCSI Fast SCSI Wide adapter device driver.
#
# RETURN VALUE:
#       0..31   =       Valid index to free element
#       32      =       No free element
#
#-----------------------------------------------------------------------------


#	.csect .asc_cntlz[PR]
#       .globl .asc_cntlz[PR]
 	S_PROLOG(asc_cntlz)
	cntlz	3, 3		# count the leading zeros and return result
 	S_EPILOG(asc_cntlz)
#	br
#        .align  2
# 	 .byte   0xdf,2 ,0xdf,0

