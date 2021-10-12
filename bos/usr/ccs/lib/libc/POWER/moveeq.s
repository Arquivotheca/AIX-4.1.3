# @(#)83	1.6  src/bos/usr/ccs/lib/libc/POWER/moveeq.s, libcasm, bos41J, 9514B 4/7/95 16:21:24
#
#  COMPONENT_NAME: (LIBCGEN) lib/c/gen 
#
#  FUNCTIONS: _moveeq, bcopy, ovbcopy, memcpy, memmove
#
#  ORIGINS: 27
#
#  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#  combined with the aggregated modules for this product)
#                   SOURCE MATERIALS
#  (C) COPYRIGHT International Business Machines Corp. 1985, 1994
#  All Rights Reserved
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#######################################################################
#
#  NAME: _moveeq
#	 bcopy   non-overlapped move
#        ovbcopy overlapped move
#
#  FUNCTION: Equal length character string move
#
#  EXECUTION ENVIRONMENT:
#  Standard register usage and linkage convention.
#  Registers used r0,r3-r12
#  Condition registers used: 0,1
#  No stack requirements.
#
#  NOTES:
#  The source string represented by R5 and R6 is moved to the target string
#  area represented by R3 and R6.
#
#  The strings may be on any address boundary and may be of any length from 0
#  through (2**31)-1 inclusive.  The strings may overlap.  The move is performed
#  in a nondestructive manner (backwards if necessary).
#
#  The addresses are treated as unsigned quantities,
#  i.e., max is 2**32-1.
#
#  The lengths are treated as unsigned quantities,
#  i.e., max is 2**31-1.
#
#  Use of this logic assumes string lengths <= 2**31-1 - Signed arithmetic
#
#  RETURN VALUE DESCRIPTION: Target string modified.
#
#
# The PL.8 entry description is ENTRY(INTEGER VALUE, CHAR(*)) NOSI.
#
#  Calling sequence: ovbcopy and bcopy
#       R3   Address of source string
#       R4   Address of target string
#       R5   Length of target and source string
#
#  Calling sequence: _moveeq, memmove, memcpy
#       R3   Address of target string
#       R4   Address of source string
#       R5   Length of source string
#
          S_PROLOG(bcopy)

	  .file	   "moveeq.s"

ENTRY(ovbcopy):
          .globl   ENTRY(ovbcopy)
	  mr       r6,r3             #  Save source address in r6
	  mr       r3,r4             #  Move target address to r3
	  mr       r4,r6             #  Move source address to r4

ENTRY(_moveeq):
          .globl   ENTRY(_moveeq)
ENTRY(memcpy):
          .globl   ENTRY(memcpy)
ENTRY(memmove):
          .globl   ENTRY(memmove)

          sri.   r0,r5,5     #1 Number of 32-byte chunks to move (L/32),
#                                 CR0 = short/long move switch.
          bgt    cr0,movelong #3/0 Branch if > 32 bytes to move.
#
# Short move (0 to 32 bytes).
#
          mtxer  r5          #1 Set move length (presuming short).
          lsx    r5,0,r4     #v Get source string (kills R5 - R12).
          stsx   r5,0,r3     #v Store it.
          br                 #0 Return.
#
# Here we check if we have overlapping strings.
# overlap if: source < target && target < source + length
#		r4   <   r3   &&   r3   <  r4    +   r5
#
movelong: cmpl   cr1,r4,r3   #1 CR1 = forward/backward switch.
	  bge	 cr1,nobackward #skip if r4 >= r3
	  a	 r10,r4,r5   # r10 = r4 + r5 (source + length)
	  cmpl	 cr1,r3,r10  # Test if long fwd move might destruct
nobackward:
          mtctr  r0          #1 CTR = num chunks to move.
          lil    r0,32       #1
          mtxer  r0          #1 XER = 32 (move length per iteration).
          rlinm. r0,r5,0,0x1f #1 R0 = remainder length.
          st     r3,24(r1)   # save target address for return
          sf     r3,r4,r3    #1 R3 = targ addr - source addr.
          blt    cr1,backward #0 B If A(source) < A(target) logically.
#
#
forward:  lsx    r5,0,r4     #8 Get 32 bytes of source.
          stsx   r5,r3,r4    #8 Store it.
          ai     r4,r4,32    #1 Increment source address.
          bdn    forward     #0 Decr count, Br if chunk(s) left to do.
#
          mtxer  r0          #1 XER = remainder length.
          lsx    r5,0,r4     #v Get the remainder string.
          stsx   r5,r3,r4    #v Store it.
          l      r3,24(r1)   # restore target start address for return
          br                 #0 Return.
#
backward: a      r4,r4,r5    #1 R4 = source addr + len
#
loopb:    ai     r4,r4,-32   #1 Decrement source address.
          lsx    r5,0,r4     #8 Get 32 bytes of source.
          stsx   r5,r3,r4    #8 Store it.
          bdn    loopb       #0 Decr count, Br if chunk(s) left to do.
#
          sf     r4,r0,r4    #1 Decrement source address.
          mtxer  r0          #1 XER = remainder length.
          lsx    r5,0,r4     #v Get the remainder string.
          stsx   r5,r3,r4    #v Store it.
          l      r3,24(r1)   # restore target start address for return
          S_EPILOG           #return.   Generate proc end table
          FCNDES(bcopy)      #Function Descriptors
.toc
.csect  ovbcopy[DS]
.globl  ovbcopy[DS]
.long   .ovbcopy
.long   TOC[t0]

.toc
.csect  _moveeq[DS]
.globl  _moveeq[DS]
.long   ._moveeq
.long   TOC[t0]

.toc
.csect  memcpy[DS]
.globl  memcpy[DS]
.long   .memcpy
.long   TOC[t0]

.toc
.csect  memmove[DS]
.globl  memmove[DS]
.long   .memmove
.long   TOC[t0]
