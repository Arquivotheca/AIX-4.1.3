# @(#)31	1.9  src/bos/usr/ccs/lib/libc/POWER/ptrgl.s, libcgen, bos411, 9428A410j 7/28/92 16:56:42
#
#  COMPONENT_NAME: (LIBCGEN) lib/c/gen 
#
#  FUNCTIONS: ptrgl
#
#  ORIGINS: 27
#
#  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#  combined with the aggregated modules for this product)
#                   SOURCE MATERIALS
#  (C) COPYRIGHT International Business Machines Corp. 1985, 1989
#  All Rights Reserved
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#-----------------------------------------------------------------------#
#
#       This routine implements the "pointer global linkage" function
#       code to allow calls by pointer to function.
#
# R2 linkage register conventions:
#       R2      TOC
#       R1      stack pointer
#       R11     address of the function descriptor
#       LR      return address
#
#   R2 routine for calling via function pointer (TOC mode).
#   Does not use BCTR.
#   Also, does not rely on caller to have Load of RTOC; 
#   compiler puts out a ZNOP after the CALL).
#
	.file	"ptrgl.s"

        .csect  ENTRY(_ptrgl[PR])
        .globl  ENTRY(_ptrgl)
        .globl  ENTRY(SPTRGL)

ENTRY(_ptrgl):
ENTRY(SPTRGL):
#
        .rename .SPTRGL,".$PTRGL"

           .set       RTCO,2 # Change to RTOC and delete when asm is fixed.
           .set       RDSA,1
           .set       RLINK,31
           .set     RENV,11     # Environment/descriptor pointer.
           .set     DLINK,16    # Disp in DSA to save rtn addr (temp).
           .set     DTOC,20     # Disp in DSA to save TOC pointer.
#    Enter here
           l        r0,0(RENV)  # Load address of target routine.
           st       RTCO,DTOC(RDSA) # Save current TOC address.
           mtctr    r0          # Move target address to Count Reg
           l        RTCO,4(RENV) # Load address of target's TOC.
           l        RENV,8(RENV) # Get environment pointer.
	   bctr		# Branch to target.
# ## Returns to caller via the LINK REG
           S_EPILOG             #generates a br followed by tag

#-----------------------------------------------------------------------#
#       common cross-language linkage interface definition
#-----------------------------------------------------------------------#

include(sys/comlink.m4)

