# @(#)27	1.7  src/bos/usr/ccs/lib/libc/POWER/udiv.s, libcmisc, bos411, 9428A410j 6/15/90 17:55:20
#
#  COMPONENT_NAME: (LIBCMISC) lib/c/misc 
#
#  FUNCTIONS: _udiv - _ud23,_umd23
#
#  ORIGINS: 27
#
#  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#  combined with the aggregated modules for this product)
#                   SOURCE MATERIALS
#  (C) COPYRIGHT International Business Machines Corp. 1985, 1989
#  All Rights Reserved
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Temporary names ----------------------------------------------
#   ### temporary .set statement located a S_PROLOG(_ud23) statement
         .globl   ENTRY(SUD23)        # Obsolete system name
         .rename  ENTRY(SUD23),"ENTRY($UD23)"

#   ### temporary .set statement located a S_PROLOG(_ud23) statement
         .globl   ENTRY(Sud23)        # Interim system name

#   ### temporary .set statement located a S_PROLOG(_umd23) statement
         .globl   ENTRY(SUMD23)        # Obsolete system name
         .rename  ENTRY(SUMD23),"ENTRY($UMD23)"

#   ### temporary .set statement located a S_PROLOG(_umd23) statement
         .globl   ENTRY(Sumd23)        # Interim system name
# Temporary names ----------------------------------------------
#
#  NAME: _ud23
#
#  FUNCTION: Unsigned division
#
#  EXECUTION ENVIRONMENT:
#  Standard register usage and linkage convention.
#  Registers used r3,r4,mq
#  Condition registers used: 0,1
#  No stack requirements.
#
#  NOTES:
#  The unsigned integer in r3 is logically divided by the unsigned integer
#  in r4.  The quotient is returned in r3.  If the input r4 value is zero then
#  the result is undefined.
#
#  RETURN VALUE DESCRIPTION: Unsigned integer in r3.
#
         S_PROLOG(_ud23)

# Temporary names ----------------------------------------------
         .set     ENTRY(SUD23),$
         .set     ENTRY(Sud23),$
# Temporary names ----------------------------------------------

         cmpl     cr1,r4,r3       # cr1 = divisor ?= dividend
         cmpi     cr0,r4,0        # cr0 = divisor ?= 2**31
         mtmq     r3              # dividend 32-63 = r3
         lil      r3,0            # dividend 0-31/quotient
         bgtr     cr1             # Return quotient=0 if divisor>dividend
         ble      cr0,CL.0        # divisor<=dividend and divisor>=2**31
         div      r3,r3,r4        # quotient=r3=r3ººmq/r4 - ignore remainder
         br                       # return quotient
                                  # divisor = 0 also takes this path
CL.0:    lil      r3,1            # quotient=r3=1
                                  # return quotient
         S_EPILOG
         FCNDES(_ud23)            #Function Descriptor

#
#  NAME: _umd23
#
#  FUNCTION: Unsigned md23ulus
#
#  EXECUTION ENVIRONMENT:
#  Standard register usage and linkage convention.
#  Registers used r0,r3,r4,mq
#  Condition registers used: 0,1
#  No stack requirements.
#
#  NOTES:
#  The unsigned integer in r3 is logically divided by the unsigned integer
#  in r4.  The remainder is returned in r3.  If the input r4 value is zero then
#  the result is undefined.
#
#  RETURN VALUE DESCRIPTION: Unsigned integer in r3.
#
         S_PROLOG(_umd23)

# Temporary names ----------------------------------------------
         .set     ENTRY(SUMD23),$
         .set     ENTRY(Sumd23),$
# Temporary names ----------------------------------------------

         cmpl     cr1,r4,r3       # cr1 = divisor ?= dividend
         mtmq     r3              # mq <- dividend
         cmpi     cr0,r4,0        # cr0 = divisor ?= 0
         bgtr     cr1             # return divisor > dividend - rem=divisor
         lil      r3,0            # extend dividend with 0
         ble      cr0,CL.1        # divisor <= dividend
         div      r0,r3,r4        # r0=r3ººmq/r4 - ignore quotient
         mfmq     r3              # r3 = remainder
         br                       # return remainder
                                  # divisor = 0 takes this path
CL.1:    sf       r3,r4,r3        # remainder=dividend-divisor
                                  # return remainder
         S_EPILOG
         FCNDES(_umd23)            #Function Descriptor
