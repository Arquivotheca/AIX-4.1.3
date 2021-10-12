# @(#)90	1.10  src/bos/usr/ccs/lib/libc/POWER/fsavres.s, libcmisc, bos411, 9428A410j 7/28/92 16:51:55
#
#  COMPONENT_NAME: (LIBCMISC) lib/c/misc 
#
#  FUNCTIONS: fsavres
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
#
	  include(sys/comlink.m4)
# Temporary names ----------------------------------------------
# Temporary statements are also located at all the entry points ----
# There are defines of "_savef3" and "_restf3".
# These labels are equal to "_savef31" and "_restf31".
# Obsolete system names
          .globl   ENTRY(SSAVEF14)
          .rename  ENTRY(SSAVEF14),"ENTRY($SAVEF14)"
          .globl   ENTRY(SSAVEF15)
          .rename  ENTRY(SSAVEF15),"ENTRY($SAVEF15)"
          .globl   ENTRY(SSAVEF16)
          .rename  ENTRY(SSAVEF16),"ENTRY($SAVEF16)"
          .globl   ENTRY(SSAVEF17)
          .rename  ENTRY(SSAVEF17),"ENTRY($SAVEF17)"
          .globl   ENTRY(SSAVEF18)
          .rename  ENTRY(SSAVEF18),"ENTRY($SAVEF18)"
          .globl   ENTRY(SSAVEF19)
          .rename  ENTRY(SSAVEF19),"ENTRY($SAVEF19)"
          .globl   ENTRY(SSAVEF20)
          .rename  ENTRY(SSAVEF20),"ENTRY($SAVEF20)"
          .globl   ENTRY(SSAVEF21)
          .rename  ENTRY(SSAVEF21),"ENTRY($SAVEF21)"
          .globl   ENTRY(SSAVEF22)
          .rename  ENTRY(SSAVEF22),"ENTRY($SAVEF22)"
          .globl   ENTRY(SSAVEF23)
          .rename  ENTRY(SSAVEF23),"ENTRY($SAVEF23)"
          .globl   ENTRY(SSAVEF24)
          .rename  ENTRY(SSAVEF24),"ENTRY($SAVEF24)"
          .globl   ENTRY(SSAVEF25)
          .rename  ENTRY(SSAVEF25),"ENTRY($SAVEF25)"
          .globl   ENTRY(SSAVEF26)
          .rename  ENTRY(SSAVEF26),"ENTRY($SAVEF26)"
          .globl   ENTRY(SSAVEF27)
          .rename  ENTRY(SSAVEF27),"ENTRY($SAVEF27)"
          .globl   ENTRY(SSAVEF28)
          .rename  ENTRY(SSAVEF28),"ENTRY($SAVEF28)"
          .globl   ENTRY(SSAVEF29)
          .rename  ENTRY(SSAVEF29),"ENTRY($SAVEF29)"
          .globl   ENTRY(SSAVEF30)
          .rename  ENTRY(SSAVEF30),"ENTRY($SAVEF30)"
          .globl   ENTRY(SSAVEF31)
          .rename  ENTRY(SSAVEF31),"ENTRY($SAVEF31)"

# Interim system names
          .globl   ENTRY(Ssavef14)
          .globl   ENTRY(Ssavef15)
          .globl   ENTRY(Ssavef16)
          .globl   ENTRY(Ssavef17)
          .globl   ENTRY(Ssavef18)
          .globl   ENTRY(Ssavef19)
          .globl   ENTRY(Ssavef20)
          .globl   ENTRY(Ssavef21)
          .globl   ENTRY(Ssavef22)
          .globl   ENTRY(Ssavef23)
          .globl   ENTRY(Ssavef24)
          .globl   ENTRY(Ssavef25)
          .globl   ENTRY(Ssavef26)
          .globl   ENTRY(Ssavef27)
          .globl   ENTRY(Ssavef28)
          .globl   ENTRY(Ssavef29)
          .globl   ENTRY(Ssavef30)
          .globl   ENTRY(Ssavef31)

# Obsolete system names
          .globl   ENTRY(SRESTF14)
          .rename  ENTRY(SRESTF14),"ENTRY($RESTF14)"
          .globl   ENTRY(SRESTF15)
          .rename  ENTRY(SRESTF15),"ENTRY($RESTF15)"
          .globl   ENTRY(SRESTF16)
          .rename  ENTRY(SRESTF16),"ENTRY($RESTF16)"
          .globl   ENTRY(SRESTF17)
          .rename  ENTRY(SRESTF17),"ENTRY($RESTF17)"
          .globl   ENTRY(SRESTF18)
          .rename  ENTRY(SRESTF18),"ENTRY($RESTF18)"
          .globl   ENTRY(SRESTF19)
          .rename  ENTRY(SRESTF19),"ENTRY($RESTF19)"
          .globl   ENTRY(SRESTF20)
          .rename  ENTRY(SRESTF20),"ENTRY($RESTF20)"
          .globl   ENTRY(SRESTF21)
          .rename  ENTRY(SRESTF21),"ENTRY($RESTF21)"
          .globl   ENTRY(SRESTF22)
          .rename  ENTRY(SRESTF22),"ENTRY($RESTF22)"
          .globl   ENTRY(SRESTF23)
          .rename  ENTRY(SRESTF23),"ENTRY($RESTF23)"
          .globl   ENTRY(SRESTF24)
          .rename  ENTRY(SRESTF24),"ENTRY($RESTF24)"
          .globl   ENTRY(SRESTF25)
          .rename  ENTRY(SRESTF25),"ENTRY($RESTF25)"
          .globl   ENTRY(SRESTF26)
          .rename  ENTRY(SRESTF26),"ENTRY($RESTF26)"
          .globl   ENTRY(SRESTF27)
          .rename  ENTRY(SRESTF27),"ENTRY($RESTF27)"
          .globl   ENTRY(SRESTF28)
          .rename  ENTRY(SRESTF28),"ENTRY($RESTF28)"
          .globl   ENTRY(SRESTF29)
          .rename  ENTRY(SRESTF29),"ENTRY($RESTF29)"
          .globl   ENTRY(SRESTF30)
          .rename  ENTRY(SRESTF30),"ENTRY($RESTF30)"
          .globl   ENTRY(SRESTF31)
          .rename  ENTRY(SRESTF31),"ENTRY($RESTF31)"

# Interim system names
          .globl   ENTRY(Srestf14)
          .globl   ENTRY(Srestf15)
          .globl   ENTRY(Srestf16)
          .globl   ENTRY(Srestf17)
          .globl   ENTRY(Srestf18)
          .globl   ENTRY(Srestf19)
          .globl   ENTRY(Srestf20)
          .globl   ENTRY(Srestf21)
          .globl   ENTRY(Srestf22)
          .globl   ENTRY(Srestf23)
          .globl   ENTRY(Srestf24)
          .globl   ENTRY(Srestf25)
          .globl   ENTRY(Srestf26)
          .globl   ENTRY(Srestf27)
          .globl   ENTRY(Srestf28)
          .globl   ENTRY(Srestf29)
          .globl   ENTRY(Srestf30)
          .globl   ENTRY(Srestf31)
# Temporary statements are also located at all the entry points ----
# Temporary names ----------------------------------------------
#
#  FILE NAME: fsavres - _savfxx,_resfxx xx=14-31
#
#  NAME: _savef14,_savef15, ... _savef31
#
#  FUNCTION: Floating point register save
#
#  EXECUTION ENVIRONMENT:
#  Standard register usage and linkage convention.
#  Registers used: stack register and fr14-fr31 depending on entry point.
#  Registered altered: none
#
#  NOTES:
#  The floating register(s) represented by the entry point (14-31) are stored
#  into the floating register save area on the stack.
#
#  RETURN VALUE DESCRIPTION: none
#

# Entry names
S_PROLOG(fsav)
          .globl   ENTRY(_savef14)
          .globl   ENTRY(_savef15)
          .globl   ENTRY(_savef16)
          .globl   ENTRY(_savef17)
          .globl   ENTRY(_savef18)
          .globl   ENTRY(_savef19)
          .globl   ENTRY(_savef20)
          .globl   ENTRY(_savef21)
          .globl   ENTRY(_savef22)
          .globl   ENTRY(_savef23)
          .globl   ENTRY(_savef24)
          .globl   ENTRY(_savef25)
          .globl   ENTRY(_savef26)
          .globl   ENTRY(_savef27)
          .globl   ENTRY(_savef28)
          .globl   ENTRY(_savef29)
          .globl   ENTRY(_savef30)
          .globl   ENTRY(_savef31)
          .globl   ENTRY(_savef3)

          .set     ENTRY(Ssavef14),$        # Temporary symbol -------
          .set     ENTRY(SSAVEF14),$        # Temporary symbol -------
ENTRY(_savef14:) stfd      14,stkfpr14(stk)
          .set     ENTRY(Ssavef15),$        # Temporary symbol -------
          .set     ENTRY(SSAVEF15),$        # Temporary symbol -------
ENTRY(_savef15:) stfd      15,stkfpr15(stk)
          .set     ENTRY(Ssavef16),$        # Temporary symbol -------
          .set     ENTRY(SSAVEF16),$        # Temporary symbol -------
ENTRY(_savef16:) stfd      16,stkfpr16(stk)
          .set     ENTRY(Ssavef17),$        # Temporary symbol -------
          .set     ENTRY(SSAVEF17),$        # Temporary symbol -------
ENTRY(_savef17:) stfd      17,stkfpr17(stk)
          .set     ENTRY(Ssavef18),$        # Temporary symbol -------
          .set     ENTRY(SSAVEF18),$        # Temporary symbol -------
ENTRY(_savef18:) stfd      18,stkfpr18(stk)
          .set     ENTRY(Ssavef19),$        # Temporary symbol -------
          .set     ENTRY(SSAVEF19),$        # Temporary symbol -------
ENTRY(_savef19:) stfd      19,stkfpr19(stk)
          .set     ENTRY(Ssavef20),$        # Temporary symbol -------
          .set     ENTRY(SSAVEF20),$        # Temporary symbol -------
ENTRY(_savef20:) stfd      20,stkfpr20(stk)
          .set     ENTRY(Ssavef21),$        # Temporary symbol -------
          .set     ENTRY(SSAVEF21),$        # Temporary symbol -------
ENTRY(_savef21:) stfd      21,stkfpr21(stk)
          .set     ENTRY(Ssavef22),$        # Temporary symbol -------
          .set     ENTRY(SSAVEF22),$        # Temporary symbol -------
ENTRY(_savef22:) stfd      22,stkfpr22(stk)
          .set     ENTRY(Ssavef23),$        # Temporary symbol -------
          .set     ENTRY(SSAVEF23),$        # Temporary symbol -------
ENTRY(_savef23:) stfd      23,stkfpr23(stk)
          .set     ENTRY(Ssavef24),$        # Temporary symbol -------
          .set     ENTRY(SSAVEF24),$        # Temporary symbol -------
ENTRY(_savef24:) stfd      24,stkfpr24(stk)
          .set     ENTRY(Ssavef25),$        # Temporary symbol -------
          .set     ENTRY(SSAVEF25),$        # Temporary symbol -------
ENTRY(_savef25:) stfd      25,stkfpr25(stk)
          .set     ENTRY(Ssavef26),$        # Temporary symbol -------
          .set     ENTRY(SSAVEF26),$        # Temporary symbol -------
ENTRY(_savef26:) stfd      26,stkfpr26(stk)
          .set     ENTRY(Ssavef27),$        # Temporary symbol -------
          .set     ENTRY(SSAVEF27),$        # Temporary symbol -------
ENTRY(_savef27:) stfd      27,stkfpr27(stk)
          .set     ENTRY(Ssavef28),$        # Temporary symbol -------
          .set     ENTRY(SSAVEF28),$        # Temporary symbol -------
ENTRY(_savef28:) stfd      28,stkfpr28(stk)
          .set     ENTRY(Ssavef29),$        # Temporary symbol -------
          .set     ENTRY(SSAVEF29),$        # Temporary symbol -------
ENTRY(_savef29:) stfd      29,stkfpr29(stk)
          .set     ENTRY(Ssavef30),$        # Temporary symbol -------
          .set     ENTRY(SSAVEF30),$        # Temporary symbol -------
ENTRY(_savef30:) stfd      30,stkfpr30(stk)
          .set     ENTRY(Ssavef31),$        # Temporary symbol -------
          .set     ENTRY(SSAVEF31),$        # Temporary symbol -------
          .set     ENTRY(_savef3),$         
ENTRY(_savef31:) stfd      31,stkfpr31(stk)
          br                         #return through link register.
          _DF(_DF_NOFRAME)
          FCNDES(_savef14,label)      #Function Descriptor
          FCNDES(_savef15,label)      #Function Descriptor
          FCNDES(_savef16,label)      #Function Descriptor
          FCNDES(_savef17,label)      #Function Descriptor
          FCNDES(_savef18,label)      #Function Descriptor
          FCNDES(_savef19,label)      #Function Descriptor
          FCNDES(_savef20,label)      #Function Descriptor
          FCNDES(_savef21,label)      #Function Descriptor
          FCNDES(_savef22,label)      #Function Descriptor
          FCNDES(_savef23,label)      #Function Descriptor
          FCNDES(_savef24,label)      #Function Descriptor
          FCNDES(_savef25,label)      #Function Descriptor
          FCNDES(_savef26,label)      #Function Descriptor
          FCNDES(_savef27,label)      #Function Descriptor
          FCNDES(_savef28,label)      #Function Descriptor
          FCNDES(_savef29,label)      #Function Descriptor
          FCNDES(_savef30,label)      #Function Descriptor
          FCNDES(_savef31,label)      #Function Descriptor


#
#  NAME: _restf14,_restf15, ... _restf31
#
#  FUNCTION: Floating point register restore
#
#  EXECUTION ENVIRONMENT:
#  Standard register usage and linkage convention.
#  Registers used: stack register and fr17-fr31 depending on entry point.
#  Registered altered: fr14-fr31 depending on entry point.
#
#  NOTES:
#  The floating register(s) represented by the entry point (14-31) are loaded
#  from the floating register save area on the stack.
#
#  RETURN VALUE DESCRIPTION: none
#

# Entry names
S_PROLOG(fres)
          .globl   ENTRY(_restf14)
          .globl   ENTRY(_restf15)
          .globl   ENTRY(_restf16)
          .globl   ENTRY(_restf17)
          .globl   ENTRY(_restf18)
          .globl   ENTRY(_restf19)
          .globl   ENTRY(_restf20)
          .globl   ENTRY(_restf21)
          .globl   ENTRY(_restf22)
          .globl   ENTRY(_restf23)
          .globl   ENTRY(_restf24)
          .globl   ENTRY(_restf25)
          .globl   ENTRY(_restf26)
          .globl   ENTRY(_restf27)
          .globl   ENTRY(_restf28)
          .globl   ENTRY(_restf29)
          .globl   ENTRY(_restf30)
          .globl   ENTRY(_restf31)
          .globl   ENTRY(_restf3)     

          .set     ENTRY(SRESTF14),$        # Temporary symbol -------
          .set     ENTRY(Srestf14),$        # Temporary symbol -------
ENTRY(_restf14:) lfd      14,stkfpr14(stk)
          .set     ENTRY(SRESTF15),$        # Temporary symbol -------
          .set     ENTRY(Srestf15),$        # Temporary symbol -------
ENTRY(_restf15:) lfd      15,stkfpr15(stk)
          .set     ENTRY(SRESTF16),$        # Temporary symbol -------
          .set     ENTRY(Srestf16),$        # Temporary symbol -------
ENTRY(_restf16:) lfd      16,stkfpr16(stk)
          .set     ENTRY(SRESTF17),$        # Temporary symbol -------
          .set     ENTRY(Srestf17),$        # Temporary symbol -------
ENTRY(_restf17:) lfd      17,stkfpr17(stk)
          .set     ENTRY(SRESTF18),$        # Temporary symbol -------
          .set     ENTRY(Srestf18),$        # Temporary symbol -------
ENTRY(_restf18:) lfd      18,stkfpr18(stk)
          .set     ENTRY(Srestf19),$        # Temporary symbol -------
          .set     ENTRY(SRESTF19),$        # Temporary symbol -------
ENTRY(_restf19:) lfd      19,stkfpr19(stk)
          .set     ENTRY(SRESTF20),$        # Temporary symbol -------
          .set     ENTRY(Srestf20),$        # Temporary symbol -------
ENTRY(_restf20:) lfd      20,stkfpr20(stk)
          .set     ENTRY(SRESTF21),$        # Temporary symbol -------
          .set     ENTRY(Srestf21),$        # Temporary symbol -------
ENTRY(_restf21:) lfd      21,stkfpr21(stk)
          .set     ENTRY(SRESTF22),$        # Temporary symbol -------
          .set     ENTRY(Srestf22),$        # Temporary symbol -------
ENTRY(_restf22:) lfd      22,stkfpr22(stk)
          .set     ENTRY(SRESTF23),$        # Temporary symbol -------
          .set     ENTRY(Srestf23),$        # Temporary symbol -------
ENTRY(_restf23:) lfd      23,stkfpr23(stk)
          .set     ENTRY(SRESTF24),$        # Temporary symbol -------
          .set     ENTRY(Srestf24),$        # Temporary symbol -------
ENTRY(_restf24:) lfd      24,stkfpr24(stk)
          .set     ENTRY(SRESTF25),$        # Temporary symbol -------
          .set     ENTRY(Srestf25),$        # Temporary symbol -------
ENTRY(_restf25:) lfd      25,stkfpr25(stk)
          .set     ENTRY(SRESTF26),$        # Temporary symbol -------
          .set     ENTRY(Srestf26),$        # Temporary symbol -------
ENTRY(_restf26:) lfd      26,stkfpr26(stk)
          .set     ENTRY(SRESTF27),$        # Temporary symbol -------
          .set     ENTRY(Srestf27),$        # Temporary symbol -------
ENTRY(_restf27:) lfd      27,stkfpr27(stk)
          .set     ENTRY(SRESTF28),$        # Temporary symbol -------
          .set     ENTRY(Srestf28),$        # Temporary symbol -------
ENTRY(_restf28:) lfd      28,stkfpr28(stk)
          .set     ENTRY(SRESTF29),$        # Temporary symbol -------
          .set     ENTRY(Srestf29),$        # Temporary symbol -------
ENTRY(_restf29:) lfd      29,stkfpr29(stk)
          .set     ENTRY(SRESTF30),$        # Temporary symbol -------
          .set     ENTRY(Srestf30),$        # Temporary symbol -------
ENTRY(_restf30:) lfd      30,stkfpr30(stk)
          .set     ENTRY(SRESTF31),$        # Temporary symbol -------
          .set     ENTRY(Srestf31),$        # Temporary symbol -------
          .set     ENTRY(_restf3),$        
ENTRY(_restf31:) lfd      31,stkfpr31(stk)
          br                         #return through link register.
          _DF(_DF_NOFRAME)
          FCNDES(_restf14,label)      #Function Descriptor
          FCNDES(_restf15,label)      #Function Descriptor
          FCNDES(_restf16,label)      #Function Descriptor
          FCNDES(_restf17,label)      #Function Descriptor
          FCNDES(_restf18,label)      #Function Descriptor
          FCNDES(_restf19,label)      #Function Descriptor
          FCNDES(_restf20,label)      #Function Descriptor
          FCNDES(_restf21,label)      #Function Descriptor
          FCNDES(_restf22,label)      #Function Descriptor
          FCNDES(_restf23,label)      #Function Descriptor
          FCNDES(_restf24,label)      #Function Descriptor
          FCNDES(_restf25,label)      #Function Descriptor
          FCNDES(_restf26,label)      #Function Descriptor
          FCNDES(_restf27,label)      #Function Descriptor
          FCNDES(_restf28,label)      #Function Descriptor
          FCNDES(_restf29,label)      #Function Descriptor
          FCNDES(_restf30,label)      #Function Descriptor
          FCNDES(_restf31,label)      #Function Descriptor
