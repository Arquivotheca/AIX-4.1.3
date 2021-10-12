# @(#)96	1.2  src/bos/diag/da/fpa/fpfail2.s, dafpa, bos411, 9428A410j 1/5/93 16:42:36
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
        .file "fpfail2.s"
#       Define a TOC entry that points to our constant data

        .toc
         T.fpdata: .tc fpdata[tc], fpdata[rw]
        .align 3


.csect .fpfail2[PR]
.globl .fpfail2[PR]

# use r6 referenced constants
        l 12, T.fpdata(2)
        cal 6, fpdata(12)


        lfd     12,0(6)              # load F12 with FPSCR
        mtfsf   0xff, 12             # initialize FPSCR
        lfd     0,8(6)               # load F0
        lfd     1,16(6)               # load F1
        lfd     3,24(6)              # load F3
        lfd     5,32(6)              # load F5
        lfd     11,40(6)             # load F11
        fm      5,1,11               # f5 <- f1*f11
        fnms    0,3,5,1              # f0 <- f3*f5-f1
        mffs    12                   # get FPSCR result
        stfd    12,60(6)             # store FPSCR
        lfd     12,64(6)             # load FPSCR to shift
        lfd     13,52(6)             # get expected FPSCR
        fcmpu   0,12,13              # compare result to expected
        bne     0, error3
        lfd     10, 72(6)            # load expected result
        fcmpu   0,0,10               # compare result to expected
        bne     0,error1             # branch on compare flag
        lfd     10, 96(6)            # load expected result
        fcmpu   0,5,10               # compare result to expected
        bne     0,error2             # branch on compare flag

# no errors, so good return code of 0

        lil     3,0                 # load 0 return code
        br

# else return code of 1 for result error, 2 for FPSCR error

error1:  lil     3,1              # load nonzero return code
                 br
error2:  lil     3,2              # load nonzero return code
                 br
error3:  lil     3,3              # load nonzero return code
                 br

# CONSTANT AREA
        .csect fpdata[rw]
        .align 3

fpdata:
      .long     0x00000000,0x82004000  # 0   FPSCR INIT
      .long     0xFFFFFFFF,0x82004001  # 8   F0
      .long     0x42474876,0xE8000000  # 16  F1
      .long     0x40240000,0x00000000  # 24  F3
      .long     0x00000000,0x00000000  # 32  F5
      .long     0x3FB99999,0x99999999  # 40  F11
      .long     0xFFFFFFFF,0x82012000  # 48  FPSCR RESULT
      .long     0x00000000,0x00000000  # 56  FPSCR temp storage for shifting
      .long     0x00000000,0x00000000  # 64
      .long     0x80000000,0x00000000  # 72  F0
      .long     0x42474876,0xE8000000  # 80  F1
      .long     0x40240000,0x00000000  # 88  F3
      .long     0x4212A05F,0x20000000  # 96  F5
      .long     0x3FB99999,0x99999999  # 104 F11
