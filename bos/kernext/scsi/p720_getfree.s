# @(#)10	1.1  src/bos/kernext/scsi/p720_getfree.s, sysxscsi, bos411, 9432A411a 7/30/94 16:58:18
#
#   COMPONENT_NAME: sysxscsi
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
        .file "p720_getfree.s"

#-----------------------------------------------------------------------------
#
# NAME: p720_getfree
#
# FUNCTION: This routine returns the index of the first free element of a list
#	    by counting the leading zeroes of the word
#
# CALL: (int)p720_getfree(word)
#       uint word;            /* 32-bit word             */
#
# EXECUTION ENVIORNMENT:
#       This service can be called on either the process or
#       interrupt level
#
#       It only page faults on the stack when under a process
#
# RETURN VALUE:
#       0..31   =       Valid index to free element
#       32      =       No free element
#
#-----------------------------------------------------------------------------


	.csect .p720_getfree[PR]
       .globl .p720_getfree[PR]
# 	S_PROLOG(p720_getfree)
	cntlz	3, 3		# count the leading zeros and return result
# 	S_EPILOG(p720_getfree)
	br
        .align  2
 	 .byte   0xdf,2 ,0xdf,0

