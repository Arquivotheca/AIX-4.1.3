# @(#)53        1.14  src/bos/usr/ccs/lib/libc/POWER/move.s, libcgen, bos411, 9428A410j 8/26/93 09:00:26
          .file    "move.s"
#
#  COMPONENT_NAME: (LIBCGEN) lib/c/gen 
#
#  FUNCTIONS: _move, $MOVE
#
#  ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1985,1993
# All Rights Reserved
# Licensed Materials - Property of IBM
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#  NAME: _move   overlapped move with pad
#
#  FUNCTION: General character string move routines
#
#  EXECUTION ENVIRONMENT:
#  Standard register usage and linkage convention.
#  Registers used r0,r3-r11
#  Condition registers used: 0,1,6,7
#  No stack requirements.
#
#  NOTES:
#  The source string represented by R5 and R6 is moved to the target string
#  area represented by R3 and R4.  If the source string is longer than the
#  target string it is truncated to the target string length.  If the source
#  string is shorter than the target string it is padded with blanks.
#
#  The stings may be on any address boundary and may be of any length from 0
#  through (2**31)-1 inclusive.  The strings may overlap.  The move is performed
#  in a nondestructive manner (backwards if necessary).
#
#  The addresses are treated as signed quantities,
#  i.e., max is 2**32-1.
#
#  The lengths are treated as signed quantities,
#  i.e., max is 2**31-1.
#
#  Use of this logic assumes string lengths <= 2**31-1 - Signed arithmetic
#
#  Calling sequence: _move
#       R3   Address of target string
#       R4   Length of target string
#       R5   Address of source string
#       R6   Length of source string
#
#
#  RETURN VALUE DESCRIPTION: Target string modified.
#

          S_PROLOG(_move)

ENTRY(Smove):
          .globl   ENTRY(Smove)
          .rename  ENTRY(Smove),"ENTRY($MOVE)"

#         doz.     r7,r6,r4          #1 Pad length = Target len -. Source len.

	  sf       r7,r6,r4	     #1 get the difference
	  srai     r8,r7,31	     #1 extend sign bit to 0xffffffff or 0
	  andc.    r7,r7,r8	     #1 AND complement of extended sign with
				     #  the difference to zero it if negative

          sf       r4,r7,r4          #1 R4 = move length.
          cmpi     cr1,r4,20         #1 Test length of move.
          cax      r0,r3,r6          #1 Pad addr = target addr + source len.
          cmpi     cr6,r7,0          #0 Save result of Pad calculation (Z/NZ),
#                                       CR bit 26 (eq) = "no pad".
          sri      r6,r4,4           #1 Number of 16 byte chunks to move (L/16)
          mtxer    r4                #1 Set move length.
          bgt      cr1,movelong      #0 Branch if > 20 bytes to move.
#----------------------------------------------------------------------
# Move section - R3 Target address, R4 Move length, R5 Source address.
#----------------------------------------------------------------------
#
# Short move; use one LSX and one STSX instruction.  Zero length
# moves come thru here.
#
          lsx      r8,0,r5           #Get the string (kills R8 - R12).
          stsx     r8,0,r3           #Store the string.
          bzr      cr6               #Return if no pad required.
          b        pad               #Go pad if required.
#
movelong:
#
# String length here is always at least 21.  Scheme is to move chunks of
# 16 bytes using LSX/STSX with R4 as the index reg and use the count
# reg as loop control.  Move uses R9 - R12.  Remainder count in R8.
#
          lil      r9,16             #Chunk length for move and remainder.
#
# here we check to see if we have overlapping strings.
# overlapping strings:  if source < target && target < source + length
#			     r5   <   r3   &&   r3   <   r5   +   r4
#
          cmpl     cr1,r5,r3         #Test if long fwd   move might distruct.
	  bge	   cr1,nobackward    #skip if r5 >= r3
	  a	   r10,r5,r4	     #r10 = r5 + r4 (source + length)
          cmpl     cr1,r3,r10        #Test if long fwd   move might distruct.
nobackward:
          mtxer    r9                #Set move length.
          rlinm.   r8,r4,0,0x0000000f #R8 = remainder,
#                                     CR bit 2 (eq) = "no remainder."
          mtctr    r6                #Set loop count - Number chunks to move.
          crand    27,26,2           #CR bit 27 = "No pad and no remainder."
          blt      cr1,backward      #B If A(source) < A(target) logically.
#
forward:
# Start chunk move at first 16 bytes of target area.
          lil      r4,0              #Beginning Index.
loopf1:   lsx      r9,r4,r5          #Get next chunk of string
          stsx     r9,r4,r3          #Store it away
          ai       r4,r4,16          #Bump index up
          bdn      loopf1            #Bump count, Br if chunk(s) left to do.
          bbtr     27                #Return if no pad and no remainder.
          bz       cr0,pad           #Go do Pad if no remainder.
#
          mtxer    r8                #Set move count for remainder.
          lsx      r9,r4,r5          #Get the string.
          stsx     r9,r4,r3          #Store the string.
          bzr      cr6               #Return if pad not required.
          b        pad               #Go pad.
#
backward:
# Start chunk move at last  16 bytes of target area.
          ai       r4,r4,-16         #Beginning Index = move length - 16
loopb1:   lsx      r9,r4,r5          #Get next chunk of string
          stsx     r9,r4,r3          #Store it away
          ai       r4,r4,-16         #Bump index down
          bdn      loopb1            #Bump count, Br if chunk(s) left to do.
          bbtr     27                #Return if no pad and no remainder.
          bz       cr0,pad           #Go do Pad if no remainder.
#
          mtxer    r8                #Set move count for remainder.
# Last move at beginning of target area.
          lsx      r9,0,r5           #Get the string.
          stsx     r9,0,r3           #Store the string.
          bzr      cr6               #Return if pad not required.
#
#----------------------------------------------------------------------
# Pad  R0 = pad address, R7 = pad length, R8-R12 killable.
#----------------------------------------------------------------------
pad:      sri.     r6,r7,4           #1 R6 = p/16.
          lil      r8,0x2020         #1 Put blanks in R8 - R11.
          cau      r8,r8,0x2020      #1
          mr       r9,r8             #1
          mr       r10,r8            #1
          mr       r11,r8            #1
          bnz      padlong           #0 Branch if pad length > 16.
          mtxer    r7                #1 Do short pad (<= 16),
          stsx     r8,0,r0           #(p+3)/4
          br                         #0
#
padlong:  mtctr    r6                #Number of 16 byte chunks to pad.
          lil      r4,16             #Number of bytes per chunk.
          mtxer    r4                #Set string length to store.
          rlinm.   r7,r7,0,0x0000000f #Pad Remainder (0-15).
loopp1:   stsx     r8,0,r0           #Pad a chunk.
          ai       r0,r0,16          #Bump address up.
          bdn      loopp1            #Bump count, Br if chunk(s) left to do.
          bzr      cr0               #Return if even block.
#
padlt16:  mtxer    r7                #Set string length to store.
          stsx     r8,0,r0           #Pad what's left.
          S_EPILOG                   #return.   Generate proc end table
          FCNDES(_move)              #Function Descriptor
