# @(#)09	1.24  src/bos/kernel/ml/POWER/start.s, sysml, bos411, 9428A410j 6/21/94 17:17:23

#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

	.file	"start.s"
	.machine "com"

#-----------------------------------------------------------------------#
#
# name: start
#
# function: This is an assembler "glue" routine which links to the
#           system initialization routine, main(). A stack is set
#           up in gpr 1, and control is transferred to main(),
#           which never returns.
#
# input state:
#	r3  =   pointer to ipl control block
#	r4  =   pointer to extended ipl control block
#
#
# output state:
#       r1  =   address of temporary stack
#       r2  =   address of kernel TOC
#       r3  =   pointer to ipl control block
#
#-----------------------------------------------------------------------#

        .set    stacksize, 8192		# startup stack area size
#
        S_PROLOG(start1)
        .using  real0,r0
        cal     r2,MSR_ME|MSR_AL(0)     # set msr = disabled, xlate off
                                        #   (machine check and alignment
                                        #    enabled only)
        mtmsr   r2
	isync

#       set up toc

        bl      label
label:
        mfspr   r6,LR                   # get link register
        .using  label,r6                # get addressibility to vcon
        l       r2,ltoc                 # address vcon
        st      r2,DATA(g_toc)          # init g_toc
	LTOC(r7, i_data, data)		# get address of interrupt structure
	st	r7,DATA(g_data)		# to low memory
#
#	move lowcore variables into global variables. After moving,
#	zero out the memory at address 0 for 256 bytes
#
	LTOC(r6, header_offset, data)
	l	r7,lowcore_header_offset(0)
	st	r7,0(r6)

	LTOC(r6, ram_disk_start, data)
	l	r7,lowcore_ram_disk_start(0)
	st	r7,0(r6)

	LTOC(r6, ram_disk_end, data)
	l	r7,lowcore_ram_disk_end(0)
	st	r7,0(r6)

	LTOC(r6, dbg_avail, data)
	l	r7,lowcore_dbg_avail(0)
	st	r7,0(r6)

	LTOC(r6, base_conf_start, data)
	l	r7,lowcore_base_conf_start(0)
	st	r7,0(r6)

	LTOC(r6, base_conf_end, data)
	l	r7,lowcore_base_conf_end(0)
	st	r7,0(r6)

	LTOC(r6, base_conf_disk, data)
	l	r7,lowcore_base_conf_disk(0)
	st	r7,0(r6)


#
#       set up stack
#
        .using  toc, r2
        l       r1,.mainstk		# get startup stack pointer in r1
        cal     r1,stacksize-stkmin(r1)
#
#       set up input to expandker
#
        l       r9,.ipl_cb		# address of IPL CB pointer
        st      r3,0(r9)		# save IPL control block address
	l	r9, .ipl_cb_ext		# address of extended IPL CB pointer
	st	r4,0(r9)
	cal	r6,ipldir(r3)		# point to ipl directory
	l	r9,rammapo(r6)		# get offset to ram bit map
	a	r9,r3,r9		# point to ram bit map
	l	r5,iplinfoo(r6)		# get offset to ipl info
	a	r5,r3,r5		# point to ipl info
	l	r5,bytesperbit(r5)	# get bytes per bit
	l	r6,mapsz(r6)		# get size of map
	rlinm	r6,r6,30,0,31		# convert to words (shift right 2)

#
# 	NB: currently, zeroing out lower 256 bytes of memory
# 	cause the kernel to die; therefore, this is temporarily
#	commented out.
#

#
# 	Zero out lower 256 bytes of memory
#
	lil	r4,256			# size
	lil	r3,0			# address
	.extern	ENTRY(bzero[PR])
	bl	ENTRY(bzero[PR])

#
#       Transfer to main
#
        .extern ENTRY(main[PR])
        ba      ENTRY(main[PR])

ltoc:   .long   TOC_ORIGIN              # address of toc (see asdef.s for TOC_ORIGIN)


        .csect  mainstk[RW]
        .space  stacksize
        .extern DATA(ipl_cb[RW])
	.extern DATA(ipl_cb_ext[RW])

ifdef(`_POWER_MP',`
#-----------------------------------------------------------------------#
#
# name: start_bs_proc
#
# function: This routine is the AIX entry point from ROS code, for
#	    boot_slave processors.
#	    It is an assembler "glue" routine which is similar to
#	    start1. A stack is set up in gpr 1, and a C-code routine,
#	    main_bs_proc(), is called for further initialization.
#	     On return from main_bs_proc(), the processor context is
#	    completely set up, and we just branch to resume().
#
# input state:
#	No parameters. Uses a global temporary stack, start_bs_stk.
#
#
# state when calling main_bs_proc:
#       r1  =   address of temporary stack
#       r2  =   address of kernel TOC
#
# never returns.
#
#-----------------------------------------------------------------------#
	.machine "ppc"

	S_PROLOG(start_bs_proc)
        cal     r2,MSR_ME(0)		# set msr = disabled, xlate off
					#   (machine check and alignment
					#    enabled only)
        mtmsr   r2

#
#       set up stack and toc pointer
#
	l	r2, DATA(g_toc)		# get g_toc
        .using  toc, r2
        l       r1,.start_bs_stk	# get startup stack pointer in r1
        cal     r1,stacksize-stkmin(r1)
	l	r31, .start_bs_param	# get parameter from init_bs_procs
	l	r3, 0(r31)		# parameter is ppda pointer
	liu	r4, 0xDEAD		# junk
	l	r5, ppda_csa(r3)	# csa has been initialized by init_flihs
	bl	ENTRY(mtsprgs)		# initialize SPRGs
	.extern ENTRY(mtsprgs)		#	nothing else changed

	bl	ENTRY(main_bs_proc)
	.extern ENTRY(main_bs_proc)

#
#	Reset start_bs_param to tell boot master that this processor has freed
#	  the boot-slave stack.
#
	lil	r0, 0
	st	r0, 0(r31)		# Reset start_bs_param

	ai	r1, r0, 1		# Make stkp unusable

	bl	ENTRY(resume)		# resume this processor wait-process
	.extern ENTRY(resume)

failed:
	b	failed			# should never get here
	FCNDES(start_bs_proc)

	.machine "com"

        .csect  start_bs_stk[RW]
        .space  stacksize
DATA(start_bs_param):
	.long	0
	.globl	DATA(start_bs_param)
') #endif _POWER_MP

toc:            .toc
.mainstk:       .tc     .mainstk[TC], mainstk[RW]
.ipl_cb:        .tc     .ipl_cb[TC], DATA(ipl_cb[RW])
.ipl_cb_ext:	.tc	.ipl_cb_ext[TC], DATA(ipl_cb_ext[RW])
TOCE(header_offset, data)
TOCE(ram_disk_start, data)
TOCE(ram_disk_end, data)
TOCE(dbg_avail, data)
TOCE(base_conf_start, data)
TOCE(base_conf_end, data)
TOCE(base_conf_disk, data)
TOCE(i_data,data)

ifdef(`_POWER_MP',`
.start_bs_stk:	.tc     .start_bs_stk[TC], start_bs_stk[RW]
		.extern	DATA(start_bs_param[RW])
.start_bs_param:.tc	.start_bs_param[TC], DATA(start_bs_param[RW])
') #endif _POWER_MP

#-----------------------------------------------------------------------#
#	include files
#-----------------------------------------------------------------------#

include(low_dsect.m4)
include(scrs.m4)
include(iplcb.m4)
include(seg.m4)
include(machine.m4)




