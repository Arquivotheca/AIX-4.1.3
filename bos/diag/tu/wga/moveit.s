# @(#)40        1.1  src/bos/diag/tu/wga/moveit.s, tu_wga, bos411, 9428A410j 3/19/93 09:48:30
#
#   COMPONENT_NAME: TU_WGA
#
#   FUNCTIONS: moveit
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#######################################################################
#
#  NAME: moveit
#
#  DESCRIPTION:
#     Fast copies WGA memory from one location to another.
#
#  INPUT:
#     1) Address of target memory to be copied to
#     2) Address of source memory to be copied from
#     3) Length of memory in pixels
#     4) x addr increment value in words
#     5) y addr increment value to next scan line
#     6) number of scan lines (in pixels) to be copied
#
#  OUTPUT:
#     The area of source in VRAM will be copied to target
#
#  RETURN:
#     None.
#
#######################################################################
#        S_PROLOG(moveit)

          .csect .moveit[PR]
          .globl .moveit[PR]

        xor     9,9,9           # clear r9
        a       12,5,9          # save off length of target string

yloop:
        a       10,3,9          # save off Address of target string
        a       11,4,9          # save off Address of source string
xloop:
        l       9,0(4)          # read adapter memory from source
        st      9,0(3)          # write adapter memory to destination
        a       4,4,6           # add x address increment
        si.     5,5,1           # decrement transfer count

# This second store is added to compensate for a hardware bug.  This
# bug causes any operation after a read to Vram to have a good chance of
# being ignored.  The second st will insure data has been written.
#
#       st      9,0(3)          # write adapter memory to destination

        a       3,3,6           # add x address increment
        bc      0xC,1,xloop     # loop until scan line is complete
                                #
        xor     9,9,9           # re-clear r9
        a       3,10,7          # add y address increment
        a       4,11,7          # add y address increment
        a       5,12,9          # restore length of target string
        si.     8,8,1           # decrement scan line count
        bc      0xC,1,yloop     # loop through all scanlines
        br                      #

        .align  2
        .byte   0xdf,2 ,0xdf,0
#return.   Generate proc end table
#       S_EPILOG                #return.   Generate proc end table
