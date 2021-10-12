# @(#)96	1.1  src/bos/diag/tu/gga/copystr.s, tu_gla, bos41J, 9515A_all 4/6/95 09:27:41
#
#   COMPONENT_NAME: tu_gla
#
#   FUNCTIONS: bytes
#		xfer
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#######################################################################
#-----------------------------------------------------------------------------#
# NAME:         CopyString                                                    #
# ASSEMBLER:    AIX                                                           #
# PURPOSE:      This routine is called by a C program to write number         #
#               of bytes to the I/O space using Load/Store instruction of the #
#               RISC/6000 instruction.  Load/Store instruction is a fastest   #
#               to send a string of data to the I/O space.                    #
# PASSED:       R3 - Target address (to be written to)                        #
#               R4 - Source address (to be read from)                         #
#               R5 - Number of bytes to write                                 #
#                                                                             #
#-----------------------------------------------------------------------------#

          .csect    .CopyString[PR]
          .globl    .CopyString[PR]

           stm       0,-128(1)            # save registers 0-31 to stack
                                          # target address on R3 save to -116(1)
                                          # src address on R4 save to -112(1)
                                          # no. of bytes on R5 save to -108(1)

#-----------------------------------------------------------------------------#
# Registers usage by this function (do not call other func. from this func.): #
# R0       = number of bytes to be transfer by caller                         #
# R1       = Stack pointer (reserved)                                         #
# R2       = Reserved                                                         #
# R3       = Target address (to be written)                                   #
# R4       = Source address (to be read)                                      #
# R5       = number of bytes to transfer to XER (25-31)                       #
# R6 - R31 = data to be sent to the Load/Store intruction                     #
# NOTE:    Maximum data to be transfered is 104 bytes per Load/Store intr.    #
#          because we are using R6 - R31 to transfer the data (26 registers,  #
#          each register is 4 bytes long; therefore bytes to be transfered    #
#          for each instruction are 104 bytes).                               #
#-----------------------------------------------------------------------------#

           or        0,5,5                # save no. of bytes to be xfer to R0

loop:
           cmpi      0,0,104              # no. of bytes to xfer > 104 bytes ?
           bgt       bytesxfer            # if > 104 bytes then xfer 104 bytes

           or        5,0,0                # remain no. of bytes to be xfer to R5
           b         loadstring           # go load the string

bytesxfer:
           lil       5,104                # load 104 bytes to be xfer (max)
                                          # 'lsx' intr. uses from regs 6-31;
                                          # therefore, 26 regs = 26 * 4 = 104

loadstring:
           mtspr     1,5                  # set up the XER bit fld (25-31)
                                          # for use the the stsx and lsx intr.
                                          # max. is 128 bytes (bits 25 - 31)

           lsx       6,0,4                # load the src string address
           stsx      6,0,3                # write src string to dest. addr.

           a         4,4,5                # update src address
           sf.       0,5,0                # remain number of bytes to be xfer
           bgt       loop                 # continue if more bytes to xfer

# next 3 intructions to restore registers 0 - 31 from stack for RSC compatiable
           lm        2,-120(1)            # restore registers 2-31 from stack
           l         0,-128(1)            # restore register 0 from stack
           l         1,-124(1)            # restore register 1 from stack

           br                             # return to the caller

