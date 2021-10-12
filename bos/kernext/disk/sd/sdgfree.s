# @(#)08	1.3  src/bos/kernext/disk/sd/sdgfree.s, sysxdisk, bos411, 9428A410j 4/30/91 09:07:22
#*****************************************************************************
# COMPONENT_NAME: (SYSXDISK)
#
# FUNCTIONS: sd_getfree
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1990, 1991
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

        .file "sdgfree.s"

#-----------------------------------------------------------------------------
#
# NAME: sd_getfree
#
# FUNCTION: This routine returns the index of the first free element of a list
#	    by counting the leading zeroes of the word
#
# CALL: (int)sd_getfree(word)
#       uint word;            /* 32-bit word             */
#
# EXECUTION ENVIORNMENT:
#       This service can be called on either the process or
#       interrupt level
#
#       It only page faults on the stack when under a process
#
# NOTES:
#       This is a specialized interface for list management for the Serial
#	Dasd Subsystem Device Driver.
#
# RETURN VALUE:
#       0..31   =       Valid index to free element
#       32      =       No free element
#
#-----------------------------------------------------------------------------


#	.csect .sd_getfree[PR]
#       .globl .sd_getfree[PR]
 	S_PROLOG(sd_getfree)
	cntlz	3, 3		# count the leading zeros and return result
 	S_EPILOG(sd_getfree)
#	br
#        .align  2
# 	 .byte   0xdf,2 ,0xdf,0

