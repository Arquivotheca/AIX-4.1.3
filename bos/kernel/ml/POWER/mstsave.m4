# @(#)78	1.15.1.5  src/bos/kernel/ml/POWER/mstsave.m4, sysml, bos411, 9428A410j 4/14/94 12:44:32
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS: NONE
#
#   ORIGINS: 27,83
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1989,1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************


#-----------------------------------------------------------------------#
#
#               Machine State Save Area -- R2 version
#		this is the assembler version of <sys/mstsave.h>
#
#               The fields for the segment registers, general purpose
#               registers, and floating point registers are aligned to
#               minimize the number of cache lines that contain them.
#               Therefore, this structure should be aligned on a cache
#               line boundary.
#
#-----------------------------------------------------------------------#

		.dsect	mstsave

mstprev:	.long	0		# previous save area
mstkjmpbuf:	.long	0		# pointer to saved context
mststackfix:	.long	0		# stack fix pointer
mstintpri:	.byte	0		# interrupt priority
mstbackt:	.byte 	0		# back-track flag
		.short	0		# reserved
mstcurid:	.long	0		# copy of curid

excp_type:      .long   0               # exception type for debugger
mstiar:         .long   0               # instruction address register
mstmsr:         .long   0               # machine state register
mstcr:          .long   0               # cond status register
mstlr:          .long   0               # link register
mstctr:         .long   0               # counter
mstxer:         .long   0               # fixed-point exception register
mstmq:		.long	0		# multiply/quotient register
msttid:		.long   0               # tid register
mstfpscr:       .long   0               # floating point status register
mstfpeu:	.byte   0               # floating point ever used
	.set	 FP_NEVER_USED, 0	# Floating point never used
	.set	 FP_USED, 1		# Floating point used
#
#     The following byte is used for floating point status information.
#
mstfpinfo:      .byte   0		# Floating point status flags
        .set     FP_IMP_INT,   0x01     # Flt-pt imprecise interrupt (IE bit)
        .set     FP_SYNC_TASK, 0x08     # Single-Thread task (MSR(FE) on)
        .set     FP_SYNC_IMP_S,   8     # shift fpinfo left: aligns w FE | IE

#     Pad so buslimit et al. are 8-byte aligned:
		.short  0

except:		.long	0		# beginning of exception structure
except1:	.long	0
except2:	.long	0
except3:	.long	0
except4:	.long	0		#
		.space  1*4             # unused padding
mstoiar:        .long   0		# iar saved in exception()
mstotoc: 	.long	0		# toc saved in exception()
mstoarg1:	.long 	0		# R3 saved  in exception()
excbranch:	.long	0		# if not NULL, address to branch
					# to on exception - so assembler
					# can avoid setjmpx overhead
mstfpscrx:	.long   0		# software extension to fpscr
mstovaddr:	.long	0		# Saved vaddr for vmexception
mstioalloc:	.long	0		# io allocation mask
mstdbat:	.space	6*4		# bat save area

#     NOTE:	Following (allocation flags, sr values) parallel the
#		type definition for adspace_t in <sys/m_types.h>
mstsralloc:     .long   0               # allocation flags for sr's
		.set	NSRS, 16	# number of segment registers
mstsr:		.space	4*NSRS		# segment registers

		.set    NGPRS, 32       # number of general purpose regs
mstgpr:		.space	4*NGPRS		# general purpose registers

		.set	NFPRS, 32	# number of floating point regs
mstfpr:		.space	8*NFPRS		# floating point registers

#-----------------------------------------------------------------------#
#	NOTE:	mstend-mstsave is used in user.m4 to compute the size
#		of the mstsave structure in the u-block.  Therefore, this
#		value MUST agree with the exact length of `sys/mstsave.h'.
#-----------------------------------------------------------------------#

mstend:	.space 0			# end of the MST

	.set	framesize, 4096		# size of MST plus interrupt stack
	.set	NUMBER_OF_FRAMES, 11  	# Number of Frames per MST stack
