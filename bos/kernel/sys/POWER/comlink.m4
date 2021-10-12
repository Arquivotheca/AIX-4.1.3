# @(#)22	1.8  src/bos/kernel/sys/POWER/comlink.m4, cmdas, bos411, 9428A410j 6/15/90 17:46:47
#
# COMPONENT_NAME: (CMDAS) Assembler and Macroprocessor 
#
# FUNCTIONS: 
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1990
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#-----------------------------------------------------------------------#
#  NAME:        comlink.m4
#
#  PLATFORM:    R2
#
#  FUNCTION:    This file defines equates for the common cross-language
#               subroutine linkage conventions.
#
#-----------------------------------------------------------------------#

#       Stack Register

        .set stk,1                      # Stack Register

#       Parameters passed on the stack

        .set stkmin, 56                 # minimum stack frame size
        .set stkp9, stkmin              # pass parameter word 9 here

#       Parameters normally in registers
#       (callee may store them here to get contiguous argument list)

        .set stkp8, 52
        .set stkp1, 24

#       Inter-module link area

        .set stktoc, 20                 # inter-module saved toc pointer
        .set stkret, 16                 # inter-module saved return addr
                                        # two reserved words
        .set stklink, 8                 # link register save area
        .set stkcond, 4                 # condition register save area
        .set stkback, 0                 # backchain to caller's frame

#       Floating point register save area - full save starts with FPR14

        .set stkfpr31, -8               # fpr 31 save area
        .set stkfpr30, -16              # fpr 30 save area
        .set stkfpr29, -24              # fpr 29 save area
        .set stkfpr28, -32              # fpr 28 save area
        .set stkfpr27, -40              # fpr 27 save area
        .set stkfpr26, -48              # fpr 26 save area
        .set stkfpr25, -56              # fpr 25 save area
        .set stkfpr24, -64              # fpr 24 save area
        .set stkfpr23, -72              # fpr 23 save area
        .set stkfpr22, -80              # fpr 22 save area
        .set stkfpr21, -88              # fpr 21 save area
        .set stkfpr20, -96              # fpr 20 save area
        .set stkfpr19, -104             # fpr 19 save area
        .set stkfpr18, -112             # fpr 18 save area
        .set stkfpr17, -120             # fpr 17 save area
        .set stkfpr16, -128             # fpr 16 save area
        .set stkfpr15, -136             # fpr 15 save area
        .set stkfpr14, -144             # fpr 14 save area

#       General purpose register save area
#
#       These are the offsets if no floating point registers
#       were saved, otherwise one must also subtract eight bytes
#       times the number of floating point registers saved.

        .set stkr31, -4                 # GPR save area
        .set stkr13, -76                # full save starts with R13


#       GPR's 0-12 are not normally saved, but would go here if they were.
#       These locations are normally used for local variables, but signal
#       delivery code on svc exit saves registers 0-12.

        .set stkr12, stkr13-4           # R12, not normally saved
        .set stkr0, -128                # R0, not normally saved
        .set stkr1, -124                # R1
        .set stkr2, -120                # R2
        .set stkr3, -116                # R3

#       The following reflect stack frames for routines that have no
#       local variables.

        .set stkpush, stkr13-4          #decr. to here to "buy" a frame
        .set stkpop, 0-stkpush          #incr. to here to release frame
#
#       Exec sets up an initial stack for execexit as described in exec.c
#       The constants below are the offsets to variables in this stack.
#
        .set exstkr0,0                  # offset of r0
        .set extstkr1,4                 # ofsset of r1
        .set extstkr31,124              # offset of r31
        .set exstklr,128                # offset of link reg
        .set exstkenv,132               # offset of environ pointer
        .set exerrno,136                # offset of errno

