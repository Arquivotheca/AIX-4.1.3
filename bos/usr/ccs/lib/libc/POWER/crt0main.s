# @(#)76	1.10  src/bos/usr/ccs/lib/libc/POWER/crt0main.s, libcgen, bos411, 9428A410j 6/2/94 09:00:51
#########################################################################
#
#  COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
#
#  FUNCTIONS: crt0
#
#  ORIGINS: 3 10 27
#
#  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#  combined with the aggregated modules for this product)
#                   SOURCE MATERIALS
#  (C) COPYRIGHT International Business Machines Corp. 1985, 1994
#  All Rights Reserved
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#########################################################################
#########################################################################
#
# FUNCTION NAME: crt0.s
#
# PLATFORM:     R2
#
# FUNCTION DESCRIPTION:
#
#       Startup stub for AIX V3 programs; exec system call enters
#       program here.  Calls main(), then passes exit() the return value.
#
#       Conditionally assembled with "profiling" defined to
#       start and stop standard profiling, and as `bsdprof' to start and
#       stop BSD profiling. "pcrt0.s" defines "profiling" and then
#       includes this file to produce the profiling version.
#
# Inputs:
#       R1 = stack frame pointer
#       R2 = TOC ptr
#       R3 = argc
#       R4 = argv
#       R5 = envp
#
#########################################################################

# XL Pascal support:
# External copies of argc and argv support parms access after
# main routine.
        .comm   p_xargc,4
        .comm   p_xargv,4

# XL Pascal support:
# External return code support - to set return code explicitly
# (as opposed to supplying return value from 'main') both the
# return-value must be set in p_xrc and the flag p_xrcfg
# must be set to a non-zero value. If the flag is not set to
# a non-zero value, the return code from 'main' will be used
# as the actual return value.
        .comm   p_xrcfg,4
        .comm   p_xrc,4

        FCNDES(__start)

ENTRY(__start):
        S_PROLOG(__start)       # Text segment starts here
                                # Standard entry point = __start

	.file	"crt0main.s"

# Set up pthread stuff. void __threads_init(void) is a
# function (written in C) which is responsible for performing
# all required threads initialization.
ifdef(`_THREAD_SAFE',`
        mr      r13, r3         # save args for main() during initialization
        mr      r14, r4
        mr      r15, r5
	.extern ENTRY(__threads_init)
	bl	ENTRY(__threads_init)
        cror    15,15,15           # nop
        mr      r3, r13         # restore args for main()
        mr      r4, r14
        mr      r5, r15
')


ifdef(`profiling', `
        mr      r13, r3         # save args for main() during initialisation
        mr      r14, r4
        mr      r15, r5

# Initilize profiling type
        l       r7,TC_prof_type(r2)
ifdef(`profispg',`
        lil     r8, +1             # -pg type profiling
')
ifdef(`profisp',`
        lil     r8, -1             # -p type profiling
')
# prof_type happens to be at offset 0 in _mondata
        st      r8,0(r7)           # Init _prof_type for mon.o and __mcount.o

# Call profiling initialisation routine:
#                                  profile whole program
        lil     r3, -1             # First param:  0xffffffff
        lil     r4, 0              # Second param: 0x00000000
        .extern ENTRY(monstartup)
        bl      ENTRY(monstartup)
        cror    15,15,15           # nop

        cmpi    cr0,r3,0           # test for zero value -OK return
        beq     monstok            # br if ok else call msg and return
monsterr:
        mr      r4,r3              # rc
ifdef(`profispg',`
        lil     r3, +1             # -pg type profiling
')
ifdef(`profisp',`
        lil     r3, -1             # -p type profiling
')
        .extern ENTRY(crt0msg)
        bl      ENTRY(crt0msg)     # crt0msg( proftype, rc );
        cror    15,15,15           # nop
        .extern ENTRY(exit)
        bl      ENTRY(exit)        # C cleanup & exit
        cror    15,15,15           # nop

        TRAP                       # exit should not return

monstok:
        mr      r3, r13            # restore args for main()
        mr      r4, r14
        mr      r5, r15
')


#       XL Pascal changes:
#       to support setting ret code either by return value or by explicit
#       call, initialize flag to null, implies use returned value.

        l       r18,TC_adata(r2)
        .using  _adata[rw],r18
        l       r7, retcflg     # get addr of return code flag
        lil     r8, 0           # get initial value for flag
        st      r8, 0(r7)       # init flag - no external retcode set

#       to save argc and argv values in external fields to support
#       access to program parms from routines other than the initial entry

        l       r9,  argc       # get addr of argc
        st      r3, 0(r9)       # p_xargc = argc

        l       r9,  argv       # get addr of argv
        st      r4, 0(r9)       # p_xargv = argv

# Initialize errno = 0 (ANSI C 4.1.3)
        l       r7, TC_errno(r2)
        st      r8, 0(r7)      # errno = 0

        .extern ENTRY(main)
        bl      ENTRY(main)     # call the main program for this process
        cror    15,15,15        # nop

#       Support return code setting by explicit call - if exretcfg is
#       not zero, use value in exretc, else (normal) use value returned
#       in register

        l       r7, retcflg     # get address of flag field
        l       r7,0(r7)        # get flag value
        cmpi    cr0,r7,0        # test for zero value
        beq     noretcd         # if not set use value as returned
        l       r8,  retcode    # get address of return code
        l       r3, 0(r8)       # get return code as set
noretcd:
        .extern ENTRY(exit)
        bl      ENTRY(exit)     # C cleanup & exit
        cror    15,15,15        # nop

        TRAP                    # exit should not return

        _DF(_DF_START,0)        # traceback entry for start of stack


ifdef(`profiling',` ',`

# For no profiling version define dummy version of __mcount mcount (for 3.1 compatibility).

        S_PROLOG(mcount)        # Dummy version for no profiling.
        br                      # return.
        _DF(_DF_NOFRAME,0)

        FCNDES(mcount)          # Descriptor

        S_PROLOG(__mcount)        # Dummy version for no profiling.
        br                      # return.
        _DF(_DF_NOFRAME,0)

        FCNDES(__mcount)          # Descriptor

')

#########################################################################
#
#       The user-level signal delivery code doesn't appear to have been
#       written yet.  One would expect it to go here...
#
#       _DF(_DF_SIGNAL,0)       # end of SIGNAL frame (see debug.h)
#       S_EPILOG
#########################################################################

#########################################################################
#       Define the symbols used as object position markers.
#########################################################################

        .csect  _text[PR]
_text:
        .globl  _text
                                ######################################
        .long   0xc11ff1cc      ### PTM AIX P11976 work around     ###
                                ######################################

#               _etext = End of .text area
        .csect  _etext[DB]
_etext:
        .globl  _etext
                                ######################################
        .byte   0xde            ### PTM AIX P11976 work around     ###
                                ######################################

#               _data = Beginning of data.
        .csect  _data[RW]
_data:
        .globl  _data
        .long   0               # for xcoff assembler

# XL Pascal changes:
        .csect  _adata[RW]
_adata:

argc:
        .long   p_xargc
argv:
        .long   p_xargv
retcflg:
        .long   p_xrcfg
retcode:
        .long   p_xrc

        .toc
TC_start:
        .tc     ENTRY(__start)[TC], ENTRY(__start)[PR]
TC_etext:
        .tc     _etext[TC], _etext[DB]
TC_adata:
        .tc     _adata[TC], _adata[RW]
        .extern errno[RW]
TC_errno:
        .tc     errno[TC], errno[RW]

        .extern _mondata[RW]
TC_prof_type:
        .tc     _prof_type[TC], _mondata[RW]

#               _edata = End of .data area
        .csect  _edata[UA]
_edata:
        .globl  _edata
                                ######################################
        .byte   0x00            ### PTM AIX P11976 work around     ###
                                ######################################

#               _end = Address of end of BSS (common area).
#               end = program-accessible last location in program

        .comm   end, 0
        .comm   _end, 0
