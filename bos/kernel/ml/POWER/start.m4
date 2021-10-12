# @(#)25        1.15  src/bos/kernel/ml/POWER/start.m4, sysml, bos411, 9428A410j 5/31/94 10:25:57

#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: start
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

################################################################################
##                                                                            ##
## NAME: start                                                                ##
##                                                                            ##
## FUNCTION: Entry point from ROS IPL code                                    ##
##                                                                            ##
## This routine gains control via a branch from the ROS IPL		      ##
## code, which passes in GPR 3 a pointer to the IPL Control		      ##
## block.								      ##
## This program MUST appear first in the kernel, because its		      ##
## job is to copy the memory image of the kernel into		  	      ##
## its proper place starting at address zero, overlaying the		      ##
## module header which preceeds the actual executable code.		      ##
## This header is present because the kernel is just an ordinary	      ##
## load module built by the ordinary AIX tools; when the ROS IPL	      ##
## code reads it from disk or diskette into address 0, the header	      ##
## is at address zero and the code (which logically belongs		      ##
## at 0) follows it.  The header is copied into a kernel data		      ##
## structure, and the entire kernel image is shifted down to		      ##
## start at 0.  This shift is accomplished by the code at		      ##
## label "start", below.						      ##
##                                                                            ##
## This job is not exactly a trivial					      ##
## task given that the executing routine must copy itself		      ##
## as well and make sure that the instruction cache and data		      ##
## don't get in each other's way while the self-modifying		      ##
## code is running.							      ##
##                                                                            ##
## Here's the plan of attack:						      ##
## 1) Branch-and-link to discover the actual address at			      ##
##    which "start" begins executing.  This will NOT be the		      ##
##    address shown in the assembly listing or load map,		      ##
##    because the code is offset by the length of the header.		      ##
## 2) Copy the module header into a data structure further		      ##
##    on in the kernel.							      ##
## 3) Copy the FIRST PART of this routine down to location 0,		      ##
##    using a loop which is located in higher addresses and		      ##
##    thus doesn't get copied yet.					      ##
## 4) Flush the caches and branch to the newly-moved routine.		      ##
## 5) Copy the rest of the kernel to the proper place, and		      ##
##    flush the caches again.						      ##
## 6) Set the front panel LEDs to blanks.				      ##
## 7) Branch to "start1", which creates a C environment for		      ##
##    the rest of kernel initialization.				      ##
##									      ##
################################################################################

.machine "com"

include(iplcb.m4)

start:
       					# N.B. Do NOT use "ENTRY(start)" because
                                        #   the string "start" is the one that
                                        #   the kernel build utility will match.

        bl      start10                 # Find out where we are, and branch
                                        #   to move loop in SECOND part
                                        #   of this routine
start25:
                                        # N.B. this label used at start10 -
                                        #   do not move

#*******************************************************************#
#   Move latter part of kernel down; this loop gains control        #
#   AFTER it has been copied down to its final position.            #
#*******************************************************************#

start30:
        lu      r6,4(r2)                # Get next 4 bytes to move
        stu     r6,4(r1)                # Move to proper loction
        cmp     cr0,r2,r0               # Check if all data moved
        blt     start30

#       Now must insure the dcache is stored in memory

	andiu.	r6,r17,PPC_MODEL 
	bne     cr0,start42		# PPC model

ifdef(`_KDB',`
#	nop first(power-pc) branches to millicode routines for early calls
#	to millicode. Patch out the PowerPC branch, and allow fall through
#	to the Power branch case for Power machines.
#	This code depends on the subsequent cache flushes to cover it.

	lis	r8,0x6000		# NOP instruction (ori 0,0,0)
	stw	r8,mulh_addr
	stw	r8,mull_addr
	stw	r8,divss_addr
	stw	r8,divus_addr
	stw	r8,quoss_addr
	stw	r8,quous_addr
') #endif _KDB

.machine "pwr"
        clcs    r5,0xe                  # Get minimum cache line size
.machine "com"
        cal     r8,imove(0)             # Start at end of move routine
        sf      r8,r5,r8                # Back up one cache line worth
start40:
.machine "pwr"
        clf     r8,r5                   # Flush line containing byte at
                                        #   r8+r5, update r8
.machine "com"
        cmp     cr0,r8,r0               # Check if done
        blt     start40
        dcs                             # Wait for memory to be updated
        ics                             # Also instruction cache
	b	start48

#	PPC model
#	Use one loop to flush dcache, another one to invalidate icache.
#	This is necessary because cache line sizes of these two caches may differ
 
start42:
	cal     r8,imove(0)
start43:				# begin flushing dcache
.machine "ppc"
	dcbf	0,r8
.machine "com"
	cmp	cr0,r8,r0
	a       r8,r8,r16
	blt	start43

	sync
	cal	r8,imove(0)
start46:				# begin invalidating icache
.machine "ppc"
	icbi	0,r8			 	
.machine "com"
	cmp	cr0,r8,r0
	a       r8,r8,r15
	blt	start46
	sync
	isync

#       Branch to "start1"
start48:
       .extern  ENTRY(start1[PR])
        ba      ENTRY(start1[PR])

#*******************************************************************#
#   N.B. This first loop FOLLOWS the second so the first half of    #
#   the move never clobbers itself!                                 #
#*******************************************************************#
imove:
        .long   low_com_start           # force a reference to low_com

startakh:
        .extern DATA(kernel_header)
        .long   DATA(kernel_header)

startalkh:
        .extern DATA(kernel_header_length)
        .long   DATA(kernel_header_length)

start10:
        mfspr   r13,LR                  # LR contains address of start10

#
#	Fix up IPL controll block pointers.  Two defined states:
#	ROS:
#		r3 - iplcb
#	ROS EMULATION:
#		r3 - -1
#		r4 - iplcb
#		r5 - extended iplcb
#
#	Change to:
#		r3 = iplcb
#		r4 = extended iplcb or 0 if not present

	cmpi	cr0, r3, -1		# check for ROS emulation flag
	beq	cr0, ros_emul		# branch if called from ROS emulation

	lil	r4, 0			# clear extended IPLCB pointer
	b	ros_done		# continue
ros_emul:
	mr	r3, r4			# r3 = iplcb pointer
	mr	r4, r5			# r4 = extended iplcb pointer
ros_done:

        ai      r13,r13,real0-start25   # r13 contains real address of low 0

include(memcheck.m4)

#       Set up to move header into kernel_header in loader - we just move
#       a lot of bytes so we always get it right!

        l       r1,startakh(r13)        # address of kernel_header
        l       r2,startalkh(r13)       # address of length of kernel_header
        lx      r2,r2,r13               # length - note use of r13 to relocate
        a       r1,r1,r13               # relocate kernel_header address
        a       r2,r1,r2                # compute stopping address
        cal     r5,-4(r0)               # initialize from pointer at -4

start11:
        lu      r8,4(r5)
        st      r8,0(r1)
        cal     r1,4(r1)
        cmp     cr0,r1,r2
        blt     start11

#	Determine machine model.  
#	If it is a PPC machine, determine instruction cache line and data cache
#	line sizes from iplcb.
#	r15: icache line size      
#	r16: dcache line size
#	r17: machine model

	l	r5,IPLINFO(r3)		# offset of ipl_info structure
	a	r5,r5,r3		# addr of ipl_info structure
	l	r17,MODEL(r5)		# model field in ipl_info structure
	andiu.	r5,r17,PPC_MODEL	# test if PPC model
	beq	cr0,start12		# not a PPC model 
	l	r5,PROCINFO(r3)		# offset of proc_info structure
	a	r5,r5,r3		# addr of proc_info structure
	l	r15,ICACHEBK(r5)	# icache block size
	l	r16,DCACHEBK(r5)	# dcache block size

#       Compute how many bytes to move in order to overlay the header at 0

start12:
	l       r1,DATA(lowcore_ram_disk_start)(r13)  # addr of start of RAM disk
        cal     r0,-4(r1)               # less 4 so we stop in time
        cal     r1,-4(0)                # put -4 in r1; first "stu" will go to 0
        ai      r2,r13,-4               # set from address -4 to r2
        cal     r14,imove-real0(0)      # compute number of bytes for
                                        #  first (short) move

#       First move this routine so that we can guarantee the integrity of cache

        andiu.	r5,r17,PPC_MODEL        # test if PPC model
	bne     cr0,start22             # PPC model 

start20:
        lu      r6,4(r2)                # get next 4 bytes to move
        stu     r6,4(r1)                # move to proper location
.machine "pwr"
        clf     0,r1                    # we can afford to flush every
                                        #   store for short move
.machine "com"
        cmp     cr0,r1,r14              # check if all data moved
        blt     start20

#       If here, the size of the TOC/XCOFF header in bytes has been moved to
#       location 0.  The kernel remains unchanged above this.

        dcs                             # following example from Appendix C
                                        #   of architecture
        ics                             # make sure we get the new values
                                        #   just moved above
        ba      start25                 # this instruction will branch to
                                        #   the absolute location of start25
start22:
        lu      r6,4(r2)                # get next 4 bytes to move
        stu     r6,4(r1)                # move to proper location
.machine "ppc"
        dcbf    0,r1                    # flush dcache block
	sync
        icbi    0,r1                    # invalidate icache block
.machine "com"
        cmp     cr0,r1,r14              # check if all data moved
        blt     start22
        sync    
        isync
        ba      start25                 
