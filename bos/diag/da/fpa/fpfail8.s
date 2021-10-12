# @(#)97	1.2  src/bos/diag/da/fpa/fpfail8.s, dafpa, bos411, 9428A410j 1/5/93 16:42:46
#
#
#   COMPONENT_NAME: DAFPA
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.file "fpfail8.s"
#       Define a TOC entry that points to our constant data

        .toc
         T.fpdata: .tc fpdata[tc], fpdata[rw]
        .align 3


.csect .fpfail8[PR]
.globl .fpfail8[PR]

# use r6 referenced constants
        l 12, T.fpdata(2)
        cal 6, fpdata(12)


        lfd     12,0(6)              # load F12 with FPSCR
        mtfsf   0xff, 12             # initialize FPSCR
        lfd     4,8(6)               # load F4
        lfd     11,16(6)              # load F11
        lfd     17,24(6)             # load F17
        lfd     20,32(6)             # load F20
        lfd     29,40(6)             # load F29
        fs      29,4,20              # f29 <- f4-f20
        fnma    17,11,29,29          # f17 <- f11*f29-f29
        mffs    12                   # get FPSCR result
        stfd    12,60(6)             # store FPSCR @ 60
        lfd     12,64(6)             # get FPSCR from 64 to shift
        lfd     13,52(6)             # get expected FPSCR
        fcmpu   0,12,13              # compare result to expected
        bne     0, error3
        lfd     10, 80(6)            # load expected result for f17
        fcmpu   0,17,10              # compare result to expected
        bne     0,error1             # branch on compare flag
        lfd     10, 88(6)            # load expected result for f29
        fcmpu   0,29,10              # compare result to expected
        bne     0,error2             # branch on compare flag

# no errors, so good return code of 0

        lil     3,0                 # load 0 return code
        br

# else return code of 1 for result error, 2 for FPSCR error

error1:  lil     3,1              # load nonzero return code
                 br               # result error
error2:  lil     3,2              # load nonzero return code
                 br               # result error
error3:  lil     3,3              # load nonzero return code
                 br               # FPSCR error

# CONSTANT AREA
        .csect fpdata[rw]
        .align 3

fpdata:
      .long     0x00000000,0x29F19048  #  0   FPSCR INIT
      .long     0xE7F0784E,0x9EE2FE54  #  8   F4    "  "
      .long     0xC11A4688,0x042F4836  # 16   F11   "  "
      .long     0x12345678,0x12345678  # 24   F17   "  "
      .long     0xDD1B0C16,0x64500000  # 32   F20   "  "
      .long     0xE7F0784E,0x9EE2FE54  # 40   F29   "  "
      .long     0xFFFFFFFF,0xEBF68048  # 48   FPSCR RESULT @ 52
      .long     0x00000000,0x00000000  # 56   temp storage for shifting
      .long     0x00000000,0x00000000  # 64   the FPSCR
      .long     0xC11A4688,0x042F4836  # 72   F11   "    "
      .long     0xE91B0C15,0xFEFF5C96  # 80   F17   "    "
      .long     0xE7F0784E,0x9EE2FE54  # 88   F29   "    "

