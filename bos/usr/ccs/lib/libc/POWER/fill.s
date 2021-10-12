# @(#)53        1.5  src/bos/usr/ccs/lib/libc/POWER/fill.s, libcmisc, bos411, 9428A410j 6/7/91 11:50:14
#
#  COMPONENT_NAME: (LIBCMISC) lib/c/misc 
#
#  FUNCTIONS: fill
#
#  ORIGINS: 27
#
#  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#  combined with the aggregated modules for this product)
#                   SOURCE MATERIALS
#  (C) COPYRIGHT International Business Machines Corp. 1988, 1989
#  All Rights Reserved
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
######################################################################
#
# fill.s.m4  -- To assemble this file type:
#                  m4 asdef.s fill.s.m4 > fill.s
#                  as.xs fill.s -o fill.o
#                  rm fill.s
#
          .align   3		# double-word alignment
          S_PROLOG(_fill)
#----------------------------------------------------------------------
# This routine fills storage with a given byte, starting with a given
# address and for a given length.
#
# The PL.8 entry description is ENTRY(CHAR(*), INTEGER VALUE) NOSI.
# Calling sequence:
#      R3   Target address
#      R4   Length
#      R5   Fill byte, replicated (e.g. x'20202020')
#
# The address and length are treated as unsigned quantities
# (max 2**32-1).
#
# Approximate Timings for this code in cycles, assuming operand
# is word-aligned, and the execution time of STSX is one cycle per
# word referenced:
#    Let n = fill length in bytes, r = residual bytes to fill, =
# mod(n, 32), and let "a/b" denote "floor division".  Then
#
#   t = 5 + (n+3)/4                     n <= 16,
#   t = 10 + (n+3)/4                    17 <= n <= 32,
#   t = 14 + 9(n/32) + (r+3)/4          n > 32.
#
#
#                          Linkage conventions
#               r0     Killed - Scratch for prologues
#               r1     Saved  - Stack Pointer
#               r2     Saved  - TOC
#               r3-r10 Killed - Arg 1-8
#               r11    Killed - ptr to fcn DSA ptr to int proc
#               r12    Killed - PL8 exception return
#               r13-31 Saved  - Non-volatile
#----------------------------------------------------------------------
#
#         
          cmpli    cr0,r4,16
          mr       r6,r5             #1 Replicate fill word.
          mr       r7,r5             #1 Replicate fill word.
          mr       r8,r5             #1 Replicate fill word.
          bgt      cr0,gt16          #0 Branch if > 16.
back:     mtxer    r4                #1 XER = length.
          stsx     r5,0,r3           #v Store fill chars.
          br                         #0 Return.
#
gt16:     sri.     r0,r4,5           #1 Number of 32-byte chunks to fill (L/32),
#                                 CR0 = short/long fill switch.
          mr       r9,r5             #1 Replicate fill word.
          mr       r10,r5            #1 Replicate fill word.
          mr       r11,r5            #1 Replicate fill word.
          mr       r12,r5            #1 Replicate fill word.
          beq      cr0,back          #0 Branch if <= 32 bytes to fill.
#
# Long fill (> 32 bytes).
#
          mtctr    r0                #1 CTR = num chunks to fill.
          lil      r0,32             #1
          mtxer    r0                #1 XER = 32 (fill length per iteration).
          rlinm    r4,r4,0,0x1f      #1 R4 = residual length.
#
loop:     stsx     r5,0,r3           #8 Store 32 bytes of fill.
          ai       r3,r3,32          #1 Increment target address.
          bdn      loop              #0 Decr count, Br if chunk(s) left to do.
#
          b        back              #0 Go fill the last few.
#
          _DF(_DF_NOFRAME)	# same as S_EPILOG but no 'br' instruction
#
# ***************************************************
# Function Descriptor
# ***************************************************
#
          FCNDES(_fill)
