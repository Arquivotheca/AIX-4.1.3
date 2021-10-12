# @(#)01	1.10  src/bos/usr/ccs/lib/libc/POWER/glink.s, libcgen, bos411, 9428A410j 7/28/92 16:53:08
#
#  COMPONENT_NAME: (LIBCGEN) lib/c/gen
#
#  FUNCTIONS: glink 
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
# 	glink.s defines global linkage code for a given routine to 
#	allow an INTER-module call.
#
# This module defines:
#
#    1.	A csect called glink[GL]. (intermodule call interface routine). 
#	The linkage editor will rename this to `routine_name[GL]' when
#	resolving a reference to an unresolved program, `routine_name'.
#
#    2.	A TOC entry to provide the address of the descriptor for the
#	actual routine.
#-----------------------------------------------------------------------#

	.file	"glink.s"

	.toc
glink:	.tc	glink[TC], glink[DS]	# toc entry - addr of descriptor for
	.extern	glink[DS]		# 	OUT-OF-MODULE routine


# R2 linkage register conventions:
#	R2  	TOC
#	R1	stack pointer
#	R0	work register - not preserved
#	LR	return address

	.csect	ENTRY(glink[GL])
	.globl	ENTRY(glink)

ENTRY(glink):
	l	r12, glink(r2)	# get address of OUT-OF-MODULE descriptor
	st	r2, stktoc(r1)	# save callers toc
	l	r0, 0(r12)	# get his entry address
	l	r2, 4(r12)	# get his toc
	mtctr	r0		# put entry address into count reg
	bctr			# return (branch) to entry address (in count)


#-----------------------------------------------------------------------#
#	If any traceback information is ever needed in this routine to 
#	mark the out-of-module call, this is a logical place for it to go.
#-----------------------------------------------------------------------#

#	_DF(_DF_GLUE)
#	Minimum traceback information supplied.
	.long	0		# marker of word of zeros.
	.long	0x000C8000	# ver 0, lang asm, global linkage bits
	.long	0		# remaining required debug bits zero


#-----------------------------------------------------------------------#
#	common cross-language linkage interface definition
#-----------------------------------------------------------------------#

include(sys/comlink.m4)
