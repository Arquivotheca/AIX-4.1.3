# @(#)14	1.2  src/bos/usr/ccs/lib/libg/POWER/dbxxx.s, libg, bos411, 9428A410j 4/2/91 17:11:55
#
# COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
#
# ORIGINS: 27
#
# This module contains IBM CONFIDENTIAL code. -- (IBM
# Confidential Restricted when combined with the aggregated
# modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

	   .file	"dbxxx.s"
# Points of entry to allow dbx to make "calls" in the user process
	   .align 2
	   S_PROLOG(__dbsubc)
	   brl
	   cror 0xf,0xf,0xf		# No-op as recognized by ld
	   b	ENTRY(__dbsubn)
# Call to linkage code.  Possible to use __dbsubc instead?
ENTRY(__dbsubg):
	   brl
	   l	r2,20(r1)		# Restore the current TOC
#
# Static dbx breakpoint to regain control of process
#
ENTRY(__dbsubn):
	   tge	  r2,r2
#
# Generate reference to fflush() so that ld will create descriptor
	   .extern ENTRY(fflush)
#
ENTRY(__dbsubf):
	   bl   ENTRY(fflush)
	   cror 0xf,0xf,0xf		# No-op as recognized by ld
	   b	ENTRY(__dbsubn)
#
# Space set aside to save environment and hold arguments
#
	   CSECT(dbxxx,RW)
	   .space 44
DATA(__dbargs):
	   .space 512
	   .globl DATA(__dbargs)
	   .globl ENTRY(__dbsubn)
	   .globl ENTRY(__dbsubg)
#
# Space set aside to save environment and hold arguments
#
	   FCNDES(__dbsubc)
	   FCNDES(__dbsubg,label)
	   FCNDES(__dbsubn,label)
