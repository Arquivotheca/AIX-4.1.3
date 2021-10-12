#  @(#)21       1.1.1.2  src/bos/kernext/disp/ped/pedmacro/memcpy64.s, pedmacro, bos411, 9428A410j 3/17/93 19:26:38
#
#   COMPONENT_NAME: PEDMACRO
#
#   FUNCTIONS: memcpyw.s
#	       memcpyb.s
#	       memcpydd.s
#		
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1991,1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#**********************************************************************

#       R3   Address of target string
#       R4   Address of source string
#       R5   Length of source string in words
#
#       RECOMMENDED USE: for writing to the i/o bus for block size >= 16 words.

        S_PROLOG(memcpyw)

        ai      r1, r1, -128    # make room on the stack
        stm     r16, 4(r1)      #    and save registers 16-31

        andil.  r7, r5,0x000F   # keep the lower 4 bits
        sri     r0, r5,4        # divide by 16
        mtctr   r0              # load number of 16-word blocks into counter
        sli     r8, r7, 2       # convert to bytes
        mtxer   r8              # final transfer amount

blockloopw:
        lm      r16, 0(r4)      # load 64 bytes
        stm     r16, 0(r3)      # store 64 bytes
        ai      r4, r4, 64      # increment source pointer
        bdn     blockloopw

remainderw:
        lsx     r16, 0, r4      # load remainder bytes
        stsx    r16, 0, r3      # store remainder bytes

        lm      r16, 4(r1)      # finish up by putting registers back
        ai      r1, r1, 128     # reset the stack pointer

        S_EPILOG                # return.   Generate proc end table
        FCNDES(memcpyw)         # Function Descriptors


#
# COMPONENT_NAME: (MIDDD) Graphics Mid-level Adapter Device Driver
#
# FUNCTIONS: memcpyw.s
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
# OBJECT CODE ONLY SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1991, 1992
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#**********************************************************************

#       R3   Address of target string
#       R4   Address of source string
#       R5   Length of source string in bytes
#       RECOMMENDED USE: for writing to the i/o bus for block size >= 64 bytes.
#
        S_PROLOG(memcpyb)

#
#
        ai      r1, r1, -128    # make room on the stack
        stm     r16, 4(r1)      #    and save registers 16-31

        andil.  r7, r5,0x003F   # keep the lower 6 bits
        sri.    r0, r5,6        # divide by 64
        mtctr   r0              # load number of 16-word blocks into counter
        mtxer   r7              # final transfer amount

blockloopb:
        lm      r16, 0(r4)      # load 64 bytes
        stm     r16, 0(r3)      # store 64 bytes
        ai      r4, r4, 64      # increment source pointer
        bdn     blockloopb

remainderb:
        lsx     r16, 0, r4      # load remainder bytes
        stsx    r16, 0, r3      # store remainder bytes

        lm      r16, 4(r1)      # finish up by putting registers back
        ai      r1, r1, 128     # reset the stack pointer

        S_EPILOG                # return.   Generate proc end table
        FCNDES(memcpyb)         # Function Descriptors


#
# COMPONENT_NAME: (MIDDD) Graphics Mid-level Adapter Device Driver
#
# FUNCTIONS: memcpydd.s
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
# OBJECT CODE ONLY SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1991, 1992
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#**********************************************************************

#       R3   Address of target string
#       R4   Address of source string
#       R5   Length of source string in bytes
#       general purpose, but will only send down in  max chunks of 32
#
#       RECOMMENDED USE: for writing to the i/o bus for block size <= 64 bytes

# Short move (0 to 32 bytes).
#
        S_PROLOG(memcpydd)

        sri.    r8, r5, 5       # Number of 32-byte chunks to move (L/32),
        oril    r0, r5, 0       # copy the length
        mtctr   r8              # load number of 8-word blocks into counter
        ble     cr0, remainder  # branch if no 8-word blocks

blockloop:
        lsi     r5, r4, 32      # Get source string (kills R5 - R12).
        stsi    r5, r3, 32      # Store it.
        ai      r4, r4, 32      # increment source pointer
        bdn     blockloop

remainder:
        andil.  r0, r0,0x001F   # keep the lower 5 bits
        mtxer   r0              # Set move length (presuming short).
        lsx     r5, 0, r4       # Get source string (kills R5 - R12).
        stsx    r5, 0, r3       # Store it.

        S_EPILOG                # return.   Generate proc end table
        FCNDES(memcpydd)        # Function Descriptors
