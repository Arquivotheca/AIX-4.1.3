# @(#)15	1.8  src/bos/kernel/ml/POWER/al_flih.m4, sysml, bos411, 9428A410j 5/17/94 03:29:51
#****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: al_flih
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
# Copyright (C) Bull S.A. 1994
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

#****************************************************************************
#                                                   
#     Alignment FLIH                               
#                                                 
# Name:  al_flih
#
# Function:
#       The Alignment FLIH is entered due to an alignment interrupt.
#       This FLIH runs with interrupts disabled. It does a partial state save 
#	in a Per Processor Descriptor Area (PPDA), handles the alignment 
#	condition and returns to the faulting instruction + 4.
#       This FLIH may cause a data storage interrupt in which case the
#       ds_flih alters the saved state so that we return to the instruction
#       that caused the alignment interrupt after we have processed the data
#       storage interrupt (we get the alignment interrupt again but do not
#       page fault again).
#
# Dependencies:
#       Any changes to state saved here requires changes to ds_flih
#       code that retrieves it.
#
#	Upon entry, GPR 15 is expected to contain the address 
#	of the PPDA
#
#	The Program Interrupt flih can make use of the lscbx
#	emulation code here simply by loading up machine state
#	to look like a lscbx alignment interrupt.
#
#
#****************************************************************************

        .set    UPD ,17   # dsisr bit (opcode 5 or 25) = 1 if update
        .set    REV ,18   # dsisr bit (opcode 21     ) = 1 if byte reversal
        .set    ALG ,21   # dsisr bit (opcode 4 or 24) = 1 if algebraic
        


ENTRY(al_flih):
	.globl	ENTRY(al_flih)

#######################################################################
#
#  Save Partial State 
#
#######################################################################

	# upon entry, r25-r31 have already been saved off, and r25
	# now contains the address of the save area

        mfspr   r27,SRR0                # get SRR0 
        mfspr   r28,SRR1                # get SRR1
        mflr    r29			# get LR
        mfcr    r30                     # get CR
        mfxer   r31			# get XER
        stm     r27, SAVE_SRR0(r25)     # save SRR0,SRR1,LR,CR,XER

ifdef(`FLIH_LEDS',`
########################################################################
#
# BRINGUP HACK - saves off current state (r3 - 31), increments the
#		 interrupt count, and diplays the count and the alignment
#		 handler identifier on the LEDs
########################################################################

	stm	r2, hack_save		# save register file
	l	r2, DATA(g_toc)(0)	# load kernel TOC
	l	r3,int_count(0)		# load interrupt count
	ai	r3,r3,1			# increment count
	st	r3, int_count(0)
        rlinm   r3,r3,20,0x00F00000	# move to LED low nibble
	oriu	r3,r3, 0x0600		# put alignment LED code in LED middle
	bl	ENTRY(write_leds)	# branch to write leds
	.extern ENTRY(write_leds)

ifdef(`MLDEBUG',`
	lil	r3, 0x0006		# trace id
	mfspr	r4, SRR0		# faulting iar
	mfspr	r5, DAR			# faulting address
	mfspr	r6, DSISR		# dsisr
	bl	ENTRY(mltrace)		# call trace hook
	.extern	ENTRY(mltrace)
',)

	lm	r2, hack_save		# restore register file
	b	end_hack		# branch past hack
	.align 2
hack_save:
	.space	128
end_hack:
')

########################################################################
#
#  Get DSISR, DAR and MSR, 
#	build index into DSISR table and jump into table
#
########################################################################

        mfspr   r26,DSISR               # r26 = DSISR
        mfspr   r27,DAR                 # r27 = DAR
        mfmsr   r28                     # r28 = MSR DR=0
        rlinm   r30,r26,24,0x1FC        # r30 = bits(15-21) of dsisr
        l       r30,dsisrtab(r30)       # load address of module
        mtlr    r30                     # set up link register = branch addr
        mtcr    r26                     # cr = dsisr
        oril    r28,r28,MSR_FP          # set fp enable bit in MSR copy
        oril    r29,r28,MSR_DR          # r29 = MSR DR=1
        br      			# jump to module 

        .align  2
########################################################################
#
#       DSISR Table
#       Indexed by bits 15-21 of DSISR
#
########################################################################
dsisrtab:
        include(dsisrtab.m4)            # dsisr branch table

        .align  2
########################################################################
#       Update Table
#       Indexed by Address Register (RA)
#               r25 = Save Area
#               r27 = DAR
########################################################################
updtab:
        include(updtab.m4)              # update table - moves DAR to RA

        .align   2 
########################################################################
#       Store Table
#       Indexed by Source Register
#               r25 = Save Area
#               r31 = Target for Data
########################################################################
stortab:
        include(stortab.m4)             # store table - moves rs to r31

        .align   2            
########################################################################
#
#       Load Table
#       Indexed by Target Register
#               r25 = Save Area
#               r31 = Data
########################################################################
loadtab:
        include(loadtab.m4)             # load table - moves r31 to rt


        .align  2
alignlo:                                # beginning of potential page
                                        # faulting alignment code

# NOTE:  The following two tables must start at address 0x7fff or
# lower.  Since optab2 is much shorter, it comes first.

        .align  2
########################################################################
#	Table for stfiwx decode.
#	Indexed by RA(I:6-10)
########################################################################
optab2:
	include(optab2.m4)		# stfiwx table

        .align  2
########################################################################
#       Floating Point Op Table
#       Indexed by Instruction(D:3,4;X:23,24) concatinated with RA(I:6-10)
#               r25 = Save Area
########################################################################
optab:
        include(optab.m4)               # floating point ops


trap:
        TRAP                            # this is the trap caused by an
                                        # invalid jump into the DSISR table

########################################################################
#
#  f_l_single
#
#       Load Floating Point Single (lfs, lfsx, lfsu, lfsux)
#               r25 = save area addr
#               r26 = DSISR 
#               r27 = DAR       
#               r28 = MSR:DR=0 
#               r29 = MSR:DR=1
#               r30 = scratch used by load_4
#               r31 = assemble data
#
########################################################################
#
# THE FOLLOWING IFDEF'ABLE MODULE HAS BEEN COMMENTED OUT AND REPLACED WITH
# A RUN-TIME MODEL CHECK VERSION
#
#f_l_single:
#ifdef(`POWER',`
#',`
##else
#        ai      r27,r27,-1              # prepare DAR for update loads
#')
#        mtmsr   r29                     # Turn on DR and enable FP
#ifdef(`POWER',`
#        lsi     r31,r27,4               # load four bytes
#',`
##else
#        bl     	load_4			# load four bytes       
#')
#        mtmsr   r28                     # Turn off DR 
#        st      r31,SAVE_WORK0(r25)	# store 4 bytes to save area
#        b      	com_float		# branch to common floating code

f_l_single:
	l	r30,syscfg_arch(0)	# load architecture type
	cmpi	0,r30,POWER_RS		# test if POWER
	bne	lfs_ppc			# else jump to powerpc
        mtmsr   r29                     # Turn on DR and enable FP
        lsi     r31,r27,4               # load four bytes
        mtmsr   r28                     # Turn off DR 
        st      r31,SAVE_WORK0(r25)	# store 4 bytes to save area
        b      	com_float		# branch to common floating code
lfs_ppc:
        ai      r27,r27,-1              # prepare DAR for update loads
        mtmsr   r29                     # Turn on DR and enable FP
	isync				# make sure DR is noticed
        bl     	load_4			# load four bytes       
        mtmsr   r28                     # Turn off DR 
	isync				# make sure DR is noticed
        st      r31,SAVE_WORK0(r25)	# store 4 bytes to save area
        b      	com_float		# branch to common floating code


########################################################################
#
#  f_l_double
#
#       Load Floating Point Double (lfd, lfdx, lfdu, lfdux)
#               r25 = save area addr
#               r26 = DSISR 
#               r27 = DAR       
#               r28 = MSR:DR=0 
#               r29 = MSR:DR=1 -> becomes scratch
#               r30 = scratch used by load_4
#               r31 = assemble data
#
########################################################################
#
# THE FOLLOWING IFDEF'ABLE MODULE HAS BEEN COMMENTED OUT AND REPLACED WITH
# A RUN-TIME MODEL CHECK VERSION
#
#f_l_double:
#ifdef(`POWER',`
#        mtmsr   r29                     # Turn on DR and enable FP
#        lsi     r30,r27,8               # load 8 bytes
#        mtmsr   r28                     # Turn off DR 
#        stm     r30,SAVE_WORK0(r25)     # store 8 bytes to save area
#',`
##else
#        ai      r27,r27,-1              # prepare DAR for update loads
#        mtmsr   r29                     # Turn on DR and enable FP
#        bl     	load_4         		# load four bytes       
#        or      r29,r31,r31             # move data to r29
#        bl     	load_4         		# load four bytes       
#        mtmsr   r28                     # Turn off DR 
#        st      r29,SAVE_WORK0(r25)	# store 4 bytes to save area
#        st      r31,SAVE_WORK1(r25) 	# store 4 bytes to save area
#')
##endif
#        b      	com_float		# branch to common floating code

f_l_double:
	l	r30,syscfg_arch(0)	# load architecture type
	cmpi	0,r30,POWER_RS		# test if POWER
	bne	lfd_ppc			# else jump to powerpc
        mtmsr   r29                     # Turn on DR and enable FP
        lsi     r30,r27,8               # load 8 bytes
        mtmsr   r28                     # Turn off DR 
        stm     r30,SAVE_WORK0(r25)     # store 8 bytes to save area
        b      	com_float		# branch to common floating code
lfd_ppc:
        ai      r27,r27,-1              # prepare DAR for update loads
        mtmsr   r29                     # Turn on DR and enable FP
	isync				# make sure DR is noticed
        bl     	load_4         		# load four bytes       
        or      r29,r31,r31             # move data to r29
        bl     	load_4         		# load four bytes       
        mtmsr   r28                     # Turn off DR 
	isync				# make sure DR is noticed
        st      r29,SAVE_WORK0(r25)	# store 4 bytes to save area
        st      r31,SAVE_WORK1(r25) 	# store 4 bytes to save area
        b      	com_float		# branch to common floating code

########################################################################
#
#  f_l_quad
#
#       Load Floating Point Quad (lfq, lfqx, lfqu, lfqux)
#               r25 = save area addr
#               r26 = DSISR -> becomes scratch -> becomes DSISR
#               r27 = DAR       
#               r28 = MSR:DR=0 
#               r29 = MSR:DR=1
#               r30 = scratch used by load_4
#               r31 = assemble data
#
########################################################################
#
# THE FOLLOWING IFDEF'ABLE MODULE HAS BEEN COMMENTED OUT AND REPLACED WITH
# A RUN-TIME MODEL CHECK VERSION
#
#f_l_quad:
#ifdef(`POWER',`
#        oril    r26,r29,0               # move 29 to 26
#        mtmsr   r26                     # Turn on DR and enable FP
#        lsi     r28,r27,16              # load 16 bytes
#        xoril   r26,r26,MSR_DR          # toggle DR in copy 
#        mtmsr   r26                     # Turn off DR and enable FP
#        stm     r28,SAVE_WORK0(r25)     # store 16 bytes to save area
#',`
##else
#        ai      r27,r27,-1              # prepare DAR for update loads
#        mtmsr   r29                     # Turn on DR and enable FP
#        bl     	load_4			# load four bytes       
#        or      r26,r31,r31             # move data to r26
#        bl     	load_4         		# load four bytes       
#        mtmsr   r28                     # Turn off DR 
#        st      r26,SAVE_WORK0(r25)	# store 4 bytes to save area
#        st      r31,SAVE_WORK1(r25)   	# store 4 bytes to save area
#        mtmsr   r29                     # Turn on DR and enable FP
#        bl     	load_4         		# load four bytes       
#        or      r26,r31,r31             # move data to r26
#        bl     	load_4         		# load four bytes       
#        mtmsr   r28                     # Turn off DR 
#        st      r26,SAVE_WORK2(r25)	# store 4 bytes to save area
#        st      r31,SAVE_WORK3(r25)	# store 4 bytes to save area
#')
##endif
#        mfcr    r26                     # get copy of DSISR again
#
#        # fall thru to com_float


f_l_quad:
	l	r30,syscfg_arch(0)	# load architecture type
	cmpi	0,r30,POWER_RS		# test if POWER
	bne	lfq_ppc			# else jump to powerpc
        oril    r26,r29,0               # move 29 to 26
        mtmsr   r26                     # Turn on DR and enable FP
        lsi     r28,r27,16              # load 16 bytes
        xoril   r26,r26,MSR_DR          # toggle DR in copy 
        mtmsr   r26                     # Turn off DR and enable FP
        stm     r28,SAVE_WORK0(r25)     # store 16 bytes to save area
        mfcr    r26                     # get copy of DSISR again
	b	com_float
lfq_ppc:
        ai      r27,r27,-1              # prepare DAR for update loads
        mtmsr   r29                     # Turn on DR and enable FP
	isync				# make sure DR is noticed
        bl     	load_4			# load four bytes       
        or      r26,r31,r31             # move data to r26
        bl     	load_4         		# load four bytes       
        mtmsr   r28                     # Turn off DR 
	isync				# make sure DR is noticed
        st      r26,SAVE_WORK0(r25)	# store 4 bytes to save area
        st      r31,SAVE_WORK1(r25)   	# store 4 bytes to save area
        mtmsr   r29                     # Turn on DR and enable FP
	isync				# make sure DR is noticed
        bl     	load_4         		# load four bytes       
        or      r26,r31,r31             # move data to r26
        bl     	load_4         		# load four bytes       
        mtmsr   r28                     # Turn off DR 
	isync				# make sure DR is noticed
        st      r26,SAVE_WORK2(r25)	# store 4 bytes to save area
        st      r31,SAVE_WORK3(r25)	# store 4 bytes to save area
        mfcr    r26                     # get copy of DSISR again

        # fall thru to com_float


########################################################################
#
#  com_float
#
#       Common Floating Point Code 
#               r26 = DSISR 
#               r30 = index into floating table -> branch addr
#
########################################################################
com_float:      
        rlinm   r30,r26,30,0x7F8	# r30 = bits(20-26) of dsisr
                                        # multiplied by 8 which are:
                                        # bits 2-4 (D-form) or 22-24 (X-form)
                                        # of opcode followed by RS/RT
       .using   low,r30                 #
        cal     r30,optab(r30)          # r30 = branch address in optab
       .drop    r30                     # which is needed if floating point
        mtlr    r30                  	# put in link register
        br     				# branch into table

########################################################################
#
#  f_s_single
#
#       Store Floating Point Single (stfs, stfsx, stfsu, stfsux)
#               r25 = save area addr
#               r26 = DSISR 
#               r27 = DAR       
#               r28 = MSR:DR=0 
#               r29 = MSR:DR=1
#               r30 = scratch used by store_4
#               r31 = assemble data
#
########################################################################
#
# THE FOLLOWING IFDEF'ABLE MODULE HAS BEEN COMMENTED OUT AND REPLACED WITH
# A RUN-TIME MODEL CHECK VERSION
#
#f_s_single:
#        mtmsr   r28                     # Turn on FP
#        b      	com_float      		# branch to common floating code
#f_s_ret:
#ifdef(`POWER',`
#',`
#        ai      r27,r27,-1              # prepare DAR for update stores
#')
#        l       r31,SAVE_WORK0(r25)	# load 4 bytes from save area
#        mtmsr   r29                     # Turn on DR 
#ifdef(`POWER',`
#        stsi    r31,r27,4               # store four bytes
#',`
#        bl     	store_4        		# store four bytes      
#')
#        mtmsr   r28                     # Turn off DR 
#        b      	update         		# go to update check
#

f_s_single:
        mtmsr   r28                     # Turn on FP
	isync				# make sure FP is noticed
        b      	com_float      		# branch to common floating code
f_s_ret:
        ai      r27,r27,-1              # prepare DAR for update stores
        l       r31,SAVE_WORK0(r25)	# load 4 bytes from save area
        mtmsr   r29                     # Turn on DR 
	isync				# make sure DR is noticed
        bl     	store_4        		# store four bytes      
        mtmsr   r28                     # Turn off DR 
	isync				# make sure DR is noticed
        b      	update         		# go to update check

########################################################################
#
#  f_s_double
#
#       Store Floating Point Double (stfd, stfdx, stfdu, stfdux)
#               r25 = save area addr
#               r26 = DSISR 
#               r27 = DAR       
#               r28 = MSR:DR=0 
#               r29 = MSR:DR=1 -> becomes scratch
#               r30 = scratch used by store_4
#               r31 = assemble data
#
########################################################################
#
# THE FOLLOWING IFDEF'ABLE MODULE HAS BEEN COMMENTED OUT AND REPLACED WITH
# A RUN-TIME MODEL CHECK VERSION
#
#f_s_double:
#        mtmsr   r28                     # Turn on FP
#        b      	com_float      		# branch to common floating code
#f_d_ret:
#ifdef(`POWER',`
#        lm      r30,SAVE_WORK0(r25)     # load 8 bytes from save area
#        mtmsr   r29                     # Turn on DR 
#        stsi    r30,r27,8               # store 8 bytes to DAR
#',`
#        ai      r27,r27,-1              # prepare DAR for update loads
#        l       r31,SAVE_WORK0(r25)	# load 4 bytes from save area
#        l       r30,SAVE_WORK1(r25)	# load 4 bytes from save area
#        mtmsr   r29                     # Turn on DR 
#        or      r29,r30,r30             # move 2nd word of data to r29
#        bl     	store_4        		# store four bytes      
#        or      r31,r29,r29             # move 2nd word of data to r31
#        bl     	store_4        		# store four bytes      
#')
#        mtmsr   r28                     # Turn off DR 
#        b      	update         		# go to update check

f_s_double:
        mtmsr   r28                     # Turn on FP
	isync				# make sure FP is noticed
        b      	com_float      		# branch to common floating code
f_d_ret:
        ai      r27,r27,-1              # prepare DAR for update loads
        l       r31,SAVE_WORK0(r25)	# load 4 bytes from save area
        l       r30,SAVE_WORK1(r25)	# load 4 bytes from save area
        mtmsr   r29                     # Turn on DR 
	isync				# make sure DR is noticed
        or      r29,r30,r30             # move 2nd word of data to r29
        bl     	store_4        		# store four bytes      
        or      r31,r29,r29             # move 2nd word of data to r31
        bl     	store_4        		# store four bytes      
        mtmsr   r28                     # Turn off DR 
	isync				# make sure DR is noticed
        b      	update         		# go to update check

########################################################################
#
#  f_s_quad
#
#       Store Floating Point Double (stfd, stfdx, stfdu, stfdux)
#               r25 = save area addr
#               r26 = DSISR 
#               r27 = DAR       
#               r28 = MSR:DR=0 
#               r29 = MSR:DR=1 -> becomes scratch
#               r30 = scratch used by store_4
#               r31 = assemble data
#
########################################################################
#
# THE FOLLOWING IFDEF'ABLE MODULE HAS BEEN COMMENTED OUT AND REPLACED WITH
# A RUN-TIME MODEL CHECK VERSION
#
#f_s_quad:
#        mtmsr   r28                     # Turn on FP
#        rlimi   r26,r26,31,0x400	# set bit 21 of DSISR (repair index)
#        rlinm   r26,r26,0,0xFFFFF7FF	# clear bit 20 of DSISR (repair index)
#        b      	com_float      		# branch to common floating code
#f_q_ret:
#ifdef(`POWER',`
#        oril    r26,r29,0               # move 29 to 26
#        lm      r28,SAVE_WORK0(r25)     # load 16 bytes from save area
#        mtmsr   r26                     # Turn on DR 
#        xoril   r26,r26,MSR_DR          # toggle DR in copy 
#        stsi    r28,r27,16              # store 16 bytes to DAR
#        mtmsr   r26                     # Turn on DR 
#',`
#        ai      r27,r27,-1              # prepare DAR for update loads
#        l       r31,SAVE_WORK0(r25)	# load 4 bytes from save area
#        l       r26,SAVE_WORK1(r25)    	# load 4 bytes from save area
#        mtmsr   r29                     # Turn on DR 
#        bl     	store_4        		# store four bytes      
#        or      r31,r26,r26             # move 2nd word of data to r31
#        bl     	store_4        		# store four bytes      
#        mtmsr   r28                     # Turn off DR 
#        l       r31,SAVE_WORK2(r25)    	# load 4 bytes from save area
#        l       r26,SAVE_WORK3(r25)    	# load 4 bytes from save area
#        mtmsr   r29                     # Turn on DR 
#        bl      store_4        		# store four bytes      
#        or      r31,r26,r26             # move 2nd word of data to r31
#        bl     	store_4        		# store four bytes      
#        mtmsr   r28                     # Turn off DR 
#')
#        mfcr    r26                     # reload DSISR copy
#        b      	update         		# go to update check
#

f_s_quad:
        mtmsr   r28                     # Turn on FP
	isync				# make sure FP is noticed
        rlimi   r26,r26,31,0x400	# set bit 21 of DSISR (repair index)
        rlinm   r26,r26,0,0xFFFFF7FF	# clear bit 20 of DSISR (repair index)
        b      	com_float      		# branch to common floating code
f_q_ret:
        ai      r27,r27,-1              # prepare DAR for update loads
        l       r31,SAVE_WORK0(r25)	# load 4 bytes from save area
        l       r26,SAVE_WORK1(r25)    	# load 4 bytes from save area
        mtmsr   r29                     # Turn on DR 
	isync				# make sure DR is noticed
        bl     	store_4        		# store four bytes      
        or      r31,r26,r26             # move 2nd word of data to r31
        bl     	store_4        		# store four bytes      
        mtmsr   r28                     # Turn off DR 
	isync				# make sure DR is noticed
        l       r31,SAVE_WORK2(r25)    	# load 4 bytes from save area
        l       r26,SAVE_WORK3(r25)    	# load 4 bytes from save area
        mtmsr   r29                     # Turn on DR 
	isync				# make sure DR is noticed
        bl      store_4        		# store four bytes      
        or      r31,r26,r26             # move 2nd word of data to r31
        bl     	store_4        		# store four bytes      
        mtmsr   r28                     # Turn off DR 
	isync				# make sure DR is noticed
        mfcr    r26                     # reload DSISR copy
        b      	update         		# go to update check

########################################################################
#
#  f_s_stfiwx
#
#       Store Floating Point as Integer Word Indexed (stfiwx)
#               r25 = save area addr
#               r26 = DSISR 
#               r27 = DAR       
#               r28 = MSR:DR=0 
#               r29 = MSR:DR=1 -> becomes scratch
#               r30 = scratch used by store_4
#               r31 = assemble data
#
########################################################################
f_s_stfiwx:
        mtmsr   r28                     # Turn on FP
	isync				# make sure FP is noticed
        rlinm   r30,r26,30,0xF8		# r30 = bits(22-26) of dsisr
					# (the FP register number)
                                        # multiplied by 8.

       .using   low,r30                 #
        cal     r30,optab2(r30)         # r30 = branch address in optab2
       .drop    r30                     # 
        mtlr    r30                  	# put in link register
        br     				# branch into table

f_stfiwx_ret:
        ai      r27,r27,-1              # prepare DAR for update loads
        l       r31,SAVE_WORK1(r25)	# load 4 bytes from save area
        mtmsr   r29                     # Turn on DR 
	isync				# make sure DR is noticed
        bl     	store_4        		# store four bytes      
        mtmsr   r28                     # Turn off DR 
	isync				# make sure DR is noticed
        b      	al_ret         		# we are done


########################################################################
#
#  do_load
#
#       Perform Load of Target Register
#               r26 = DSISR             
#               r30 = scratch
#               r31 = data
########################################################################
do_load:
        rlinm   r30,r26,30,0xF8		# r30 = bits(6-10) of opcode, which
                                        # is RT * 8
       .using   low,r30                 #
        cal     r30,loadtab(r30)        #
       .drop    r30                     #
        mtlr    r30                     # LR = branch addr in loadtab
        brl    				# branch into load table
        b       update   	        # branch to update check


########################################################################
#
#  l_word_u
#
#       Handle all load words with update. (lwzu, lwzux)
#	Also handles External Control Word In Indexed (ecwix) which can
# 	be treated just like a load word as long as we clear the update bit.
#       Test for rt = ra, if so, clear update bit (ra = 0 is handled in updtab)
#               r30 = scratch
#               r31 = scratch
#
########################################################################
l_word_u:
        rlinm   r30,r26,27,0x1F         # r30 = rt
        rlinm   r31,r26,0,0x1F          # r31 = ra
        cmp     0,r30,r31               # test for ra = rt
        bne     l_word   	        # go to l_word (must update)
ext_word_in:				# Notice that eciwx is handled here
        crandc  UPD,UPD,UPD             # clear update bit in cr
        
        # fall through to l_word

########################################################################
#
#  l_word
#
#       Handle all load words (lwz, lwzx, lwbrx, lwarx)
#               r25 = Save area
#               r26 = DSISR
#               r27 = DAR               
#               r28 = MSR:DR=0
#               r29 = MSR:DR=1
########################################################################
l_word:
        ai      r27,r27,-1              # decrement DAR for updates
        mtmsr   r29                     # turn on DR
	isync				# make sure DR is noticed
        bl      load_4 		        # jump to load 4, loads into r31
        mtmsr   r28                     # turn off DR
	isync				# make sure DR is noticed
        bcl     true,REV,reverse        # reverse bytes if necessary
        b       do_load 	        # perform the load

########################################################################
#
#  l_half_u
#
#       Handle all load half words with update. (lhzu, lhzux, lhau, lhaux)
#       Test for rt = ra, if so, clear update bit (ra = 0 is handled in updtab)
#               r30 = scratch
#               r31 = scratch
#
########################################################################
l_half_u:
        rlinm   r30,r26,27,0x1F		# r30 = rt
        rlinm   r31,r26,0,0x1F		# r31 = ra
        cmp     0,r30,r31               # test for ra = rt
        bne     l_half    	        # go to l_half (must update)
        crandc  UPD,UPD,UPD             # clear update bit in cr
        
        # fall through to l_half

########################################################################
#
#  l_half
#
#       Handle all load half words. (lhz, lhzx, lha, lhax, lhbrx)
#               r25 = Save area
#               r26 = DSISR
#               r27 = DAR               
#               r28 = MSR:DR=0
#               r29 = MSR:DR=1
#               r30 = scratch
#               r31 = assemble data
#
########################################################################
l_half:
        ai      r27,r27,-1              # decrement DAR for updates
        mtmsr   r29                     # turn on DR
	isync				# make sure DR is noticed
        lbzu    r30,1(r27)              # load first byte
        rlinm   r31,r30,8,0xFF00	# insert as middle low byte 
        lbzu    r30,1(r27)              # load second byte
        or      r31,r31,r30             # insert as low byte
        mtmsr   r28                     # turn off DR
	isync				# make sure DR is noticed
        bcl     true,REV,reverse_half   # reverse bytes if necessary
        bcl     true,ALG,algebra        # branch if algebraic
        b       do_load   	        # perform the load

########################################################################
#
#  load_4
#
#       Load 4 bytes from DAR 
#               r27 = DAR               
#               r30 = scratch
#               r31 = target for data
########################################################################
load_4:
        lbzu    r30,1(r27)              # load first byte
        rlinm   r31,r30,24,0xFF000000	# move to the high byte position
        lbzu    r30,1(r27)              # load second byte
        rlimi   r31,r30,16,0xFF0000	# insert the middle high byte 
        lbzu    r30,1(r27)              # load third byte
        rlimi   r31,r30,8,0xFF00	# insert the middle low byte 
        lbzu    r30,1(r27)              # load fourth byte
        or      r31,r31,r30             # insert low byte
        br      			# return

########################################################################
#
#  store_4
#
#       Store 4 bytes to *DAR 
#               r27 = DAR               
#               r30 = scratch
#               r31 = data
########################################################################
store_4:
        rlinm   r30,r31,8,0xFF		# move high byte to low position
        stbu    r30,1(r27)              # store byte to *DAR
        rlinm   r30,r31,16,0xFF		# move middle high byte to low position
        stbu    r30,1(r27)              # store byte to *DAR
        rlinm   r30,r31,24,0xFF		# move middle low byte to low position
        stbu    r30,1(r27)              # store byte to *DAR
        stbu    r31,1(r27)              # store low byte to *DAR
        br      			# return

########################################################################
#
#  s_word
#
#       Handle all store words. (stw, stwu, stwx, stw, stwbrx)
#       Also handles External Control Word Out Indexed (ecowx) since it
#	can be treated just like a store word as long as we clear the
#	update bit.
#               r25 = Save area
#               r26 = DSISR
#               r27 = DAR               
#               r28 = MSR:DR=0
#               r29 = MSR:DR=1
#               r30 = scratch
########################################################################
ext_word_out:				# Notice that ecowx is handled here
        crandc  UPD,UPD,UPD             # clear update bit in cr
s_word:
        rlinm   r30,r26,30,0xF8		# r30 = bits(6-10) of opcode
                                        # = Rs field multiplied by 8
       .using   low,r30                 #
        cal     r30,stortab(r30)        # r30 = branch addr in stortab for rs
       .drop    r30                     #
        mtlr    r30                     # LR = branch address in stortab
        brl     			# set r31 to original value of rs
        bcl     true,REV,reverse        # reverse the bytes if necessary
        ai      r27,r27,-1              # decrement DAR for update stores
        mtmsr   r29                     # turn on DR
	isync				# make sure DR is noticed
        bl      store_4                 # jump to store 4, stores r31 to DAR
        mtmsr   r28                     # turn off DR
	isync				# make sure DR is noticed
        b       update                  # branch to update check

########################################################################
#
#  s_half
#
#       Handle all store half words. (sth, sthx, sthu, sthux, sthbrx)
#               r25 = Save area
#               r26 = DSISR
#               r27 = DAR               
#               r28 = MSR:DR=0
#               r29 = MSR:DR=1
#               r30 = scratch
########################################################################
s_half:
        rlinm   r30,r26,30,0xF8		# r30 = bits(6-10) of opcode
                                        # = Rs field multiplied by 8
       .using   low,r30                 #
        cal     r30,stortab(r30)        # r30 = branch addr in stortab for rs
       .drop    r30                     #
        mtlr    r30   		 	# LR = branch address in stortab
        brl    				# set r31 to original value of rs

        bcl     true,REV,reverse_half   # reverse the bytes if necessary
        ai      r27,r27,-1              # decrement DAR for update stores
        mtmsr   r29                     # turn on DR
	isync				# make sure DR is noticed
        rlinm   r30,r31,24,0xFF		# move middle low byte to low position
        stbu    r30,1(r27)              # store byte to *DAR
        stbu    r31,1(r27)              # store low byte to *DAR
        mtmsr   r28                     # turn off DR
	isync				# make sure DR is noticed
        b       update   	        # branch to update check

########################################################################
#
#  s_cond
#
#       Store Conditional. (stwcx.)
#               r25 = save area addr
#               r30 = scratch
#               r31 = XER
#
########################################################################
s_cond:
        l       r30,SAVE_CR(r25)  	# load copy of CR from save area
        l       r31,SAVE_XER(r25)  	# load XER from save area
        rlinm   r30,r30,0,0x0FFFFFFF	# mask off CR0
	rlimi	r30,r31,29,0x10000000   # insert the correct SO bit from XER
        st      r30,SAVE_CR(r25)    	# store modified copy of CR back
        stwcx.  r30,r0,r25		# perform dummy store conditional to
                                        # clear any outstanding reservation
        b       al_ret  	        # get out

########################################################################
#
#  l_mul
#
#       Load Multiple. (lmw)
#               r26 = DSISR     
#               r30 = byte count
#               r31 = scratch
#
########################################################################
l_mul:
        rlinm   r31,r26,27,0x1F		# r31 = bits(6-10) of opcode, RT
        sfi     r30,r31,32              # compute byte count ((32 - RT) * 4)
        rlinm   r30,r30,2,0xFFFFFFFC	# multiply by 4
	lil	r31,-1			# load 31 with invalid RB
        b      	com_lm     		# branch to common load multiple code

########################################################################
#
#  l_str_ind
#
#       Load String Indexed. (lswx)
#               r25 = save area addr
#               r30 = byte count
#               r31 = scratch
########################################################################
l_str_ind:
        l       r31,SAVE_XER(r25) 	# load XER from save area
        andil.  r30,r31,0x7F            # mask off to get byte count
        l       r31,SAVE_SRR0(r25)	# load SRR0 address from save area
        mtmsr   r29                     # turn on DR
	isync				# make sure DR is noticed
        l       r31,0(r31)              # load original instruction
        mtmsr   r28                     # turn off DR
	isync				# make sure DR is noticed
        rlinm   r31,r31,24,0xF8		# pull out RB from I:16-20 (RB * 8)
        b    	com_lm         		# branch to common load multiple code

########################################################################
#
#  l_str_imm
#
#       Load String Immediate. (lswi)
#               r25 = save area addr
#               r28 = MSR:DR=0
#               r29 = MSR:DR=1
#               r30 = byte count
#               r31 = scratch
########################################################################
l_str_imm:
        l       r31,SAVE_SRR0(r25)	# load SRR0 address from save area
        mtmsr   r29                     # turn on DR
	isync				# make sure DR is noticed
        l       r31,0(r31)              # load original instruction
        mtmsr   r28                     # turn off DR
	isync				# make sure DR is noticed
        rlinm   r30,r31,21,0x1F		# pull out byte count from I:16-20
        cmpi    0,r30,0                 # see if byte count is 0
	lil	r31,-1			# load 31 with invalid RB
        bne     com_lm         		# if not go to common load multiple code
        ai      r30,r30,32              # else set byte count to 32 

        # fall through to common load multiple code

########################################################################
#
#  com_lm
#
#       Common Load Multiple Code
#               r25 = save area addr
#               r26 = DSISR --> becomes scratch
#               r27 = DAR       
#               r28 = MSR:DR=0 --> becomes RT * 8
#               r29 = MSR:DR=1
#               r30 = byte count
#               r31 = on entry contains RB*8 --> assemble data
########################################################################
com_lm:
        st      r31,SAVE_WORK3(r25)     # save RB
        andil.  r28,r30,0x3             # save remainder of div by 4
        st      r28,SAVE_WORK0(r25)     # save remainder
        rlinm   r28,r26,30,0xF8		# r28 = bits(6-10) of opcode, RT*8
        rlinm   r26,r26,3,0xF8		# r26 = bits(11-15) of opcode, RA*8
        st      r26,SAVE_WORK1(r25)     # save RA
ifdef(`POWER',`
',`
#else
        ai      r27,r27,-1              # prepare DAR for update loads
')
        rlinm   r30,r30,0,0xFFFFFFFC    # get largest multiple of 4 of bytecount
        a       r30,r30,r27             # r30 = address of last word load
        st      r30,SAVE_WORK2(r25)     # save address
next_rt:
        cmp     0,r30,r27               # if we've done the last word load
        beq     l_remainder     	# go do remainder
        mtmsr   r29                     # turn on DR
	isync				# make sure DR is noticed
        xoril   r29,r29,MSR_DR          # toggle DR in copy 
ifdef(`POWER',`
        lsi     r31,r27,4               # load 4 bytes
        ai      r27,r27,4               # increment DAR
',`
#else
        bl     	load_4         		# go get 4 bytes
')
        mtmsr   r29                     # turn off DR
	isync				# make sure DR is noticed
        xoril   r29,r29,MSR_DR          # toggle DR in copy 
        l       r26,SAVE_WORK1(r25)     # re-load RA
	cmpli	cr1,r26,0		# see if RA was 0
        cmp     0,r26,r28               # if RA == RT
 	beq	cr1,l_RA_0_a		# then RA==0, OK to load
        beq     skip            	# then skip this load
l_RA_0_a:
        l       r26,SAVE_WORK3(r25)     # re-load RB
        cmp     0,r26,r28               # if RB == RT
        beq     skip            	# then skip this load
       .using   low,r28                 #
        cal     r26,loadtab(r28)        #
       .drop    r28                     #
        mtlr   	r26                  	# LR = branch addr in loadtab
        brl    				# branch into load table
skip:
        l       r30,SAVE_WORK2(r25)  	# reload address of last word
        ai      r28,r28,1*8             # increment RT
        cmpi    0,r28,31*8              # if RT <= 31 
        ble     next_rt        		# then go to next_rt
        ai      r28,r28,-32*8           # else perform mod-32
        b      	next_rt       		# then go to next_rt

l_remainder:
        l       r26,SAVE_WORK1(r25)     # re-load RA
        cmpli   cr1,r26,0               # if RA == 0
        cmp     0,r26,r28               # if RA == RT
	beq	cr1, l_RA_0_b		# then keep going
        beq     al_ret          	# then get out
l_RA_0_b:
        l       r26,SAVE_WORK3(r25)     # re-load RB
        cmp     0,r26,r28               # if RB == RT
        beq     al_ret          	# then get out
        l       r26,SAVE_WORK0(r25)     # load the remainder
        cmpi    0,r26,0                 # if remainder == 0
        beq     al_ret          	# then get out  
        mtmsr   r29                     # turn on DR
	isync				# make sure DR is noticed
        xoril   r29,r29,MSR_DR          # toggle DR in copy 
ifdef(`POWER',`
        mtxer   r26       	  	# put remainder in XER
        lsx     r31,0,r27               # load remainder
',`
#else
        lbzu    r30,1(r27)              # load byte
        rlinm   r31,r30,24,0xFF000000   # insert into high position     
        cmpi    0,r26,1                 # if remainder == 1
        beq     load_last       	# then go load_last
        lbzu    r30,1(r27)              # load byte
        rlimi   r31,r30,16,0xFF0000	# insert into middle high position      
        cmpi    0,r26,2                 # if remainder == 2
        beq     load_last       	# then go load_last
        lbzu    r30,1(r27)              # load byte
        rlimi   r31,r30,8,0xFF00	# insert into middle low position       
')
load_last:
        mtmsr   r29                     # turn off DR
	isync				# make sure DR is noticed
       .using   low,r28                 #
        cal     r30,loadtab(r28)        #
       .drop    r28                     #
        mtlr   	r30                  	# LR = branch addr in loadtab
        brl    				# branch into load table
        b      	al_ret			# get out
        

########################################################################
#
#  s_mul
#
#       Store Multiple. (stmw)
#               r26 = DSISR     
#               r30 = byte count
#               r31 = scratch
#
########################################################################
s_mul:
        rlinm   r31,r26,27,0x1F		# r31 = bits(6-10) of opcode, RT
        sfi     r30,r31,32              # 
        rlinm   r30,r30,2,0xFFFFFFFC    # compute byte count ((32 - RT) * 4)
        b      	com_sm         		# branch to common store multiple code

########################################################################
#
#  s_str_ind
#
#       Store String Indexed. (stswx)
#               r25 = save area addr
#               r30 = byte count
#               r31 = scratch
########################################################################
s_str_ind:
        l       r31,SAVE_XER(r25)  	# load XER from save area
        andil.  r30,r31,0x7f            # mask off to get byte count
        b      	com_sm  		# branch to common store multiple code

########################################################################
#
#  s_str_imm
#
#       Store String Immediate. (stswi)
#               r25 = save area addr
#               r28 = MSR:DR=0
#               r29 = MSR:DR=1
#               r30 = byte count
#               r31 = scratch
########################################################################
s_str_imm:
        l       r31,SAVE_SRR0(r25)	# load SRR0 address from save area
        mtmsr   r29                     # turn on DR
	isync				# make sure DR is noticed
        l       r31,0(r31)              # load original instruction
        mtmsr   r28                     # turn off DR
	isync				# make sure DR is noticed
        rlinm   r30,r31,21,0x1F		# pull out byte count from I:16-20
        cmpi    0,r30,0                 # see if byte count is 0
        bne     com_sm         		# if not go to common store mul code
        ai      r30,r30,32              # else set byte count to 32 

        # fall through to common store multiple code

########################################################################
#
#  com_sm
#
#       Common Store Multiple Code
#               r25 = save area addr
#               r26 = DSISR --> becomes address of last word store
#               r27 = DAR       
#               r28 = MSR:DR=0 --> becomes scratch
#               r29 = MSR:DR=1
#               r30 = byte_count 
#               r31 = source data
########################################################################
com_sm:
        andil.  r31,r30,0x3             # save remainder of div by 4
        st      r31,SAVE_WORK0(r25)     # save remainder
        rlinm   r28,r26,30,0xF8         # r28 = bits(6-10) of opcode, RS*8
ifdef(`POWER',`
',`
        ai      r27,r27,-1              # prepare DAR for update stores
')
        rlinm   r30,r30,0,0xFFFFFFFC    # get largest multiple of 4 of bytecount
        a       r26,r30,r27             # r26 = address after last word store
next_rs:
       .using   low,r28                 #
        cal     r31,stortab(r28)        #
       .drop    r28                     #
        mtlr    r31    	                # LR = branch addr in store table
        brl    				# branch into store table, moves data
                                        # from RS to R31
        cmp     0,r26,r27               # if we've store the last word
        beq     s_remainder     	# go do remainder
        mtmsr   r29                     # turn on DR
	isync				# make sure DR is noticed
        xoril   r29,r29,MSR_DR          # toggle DR in copy 
ifdef(`POWER',`
        stsi    r31,r27,4               # store 4 bytes
        ai      r27,r27,4               # increment DAR
',`
        bl     	store_4        		# go get 4 bytes
')
        mtmsr   r29                     # turn off DR
	isync				# make sure DR is noticed
        xoril   r29,r29,MSR_DR          # toggle DR in copy 
        ai      r28,r28,1*8             # increment RS
        cmpi    0,r28,31*8              # if RT <= 31 
        ble     next_rs        		# then go to next_rs
        ai      r28,r28,-32*8           # else perform mod-32
        b      	next_rs        		# then go to next_rt

s_remainder:
        l       r26,SAVE_WORK0(r25)     # load remainder
        cmpi    0,r26,0                 # if remainder == 0
        beq     al_ret          	# then get out  
        mtmsr   r29                     # turn on DR
	isync				# make sure DR is noticed
        xoril   r29,r29,MSR_DR          # toggle DR in copy 
ifdef(`POWER',`
        mtxer   r26 			# move remainder to XER
        stsx    r31,0,r27               # store remainder
',`
        rlinm   r30,r31,8,0xFF		# move high byte to low position        
        stbu    r30,1(r27)              # store byte
        cmpi    0,r26,1                 # if remainder == 1
        beq     s_return        	# then get out
        rlinm   r30,r31,16,0xFF		# move middle high byte to low position 
        stbu    r30,1(r27)              # store byte
        cmpi    0,r26,2                 # if remainder == 2
        beq     s_return        	# then get out
        rlinm   r30,r31,24,0xFF		# move middle low byte to low position  
        stbu    r30,1(r27)              # store byte
')
s_return:
        mtmsr   r29                     # turn on DR
	isync				# make sure DR is noticed
        xoril   r29,r29,MSR_DR          # toggle DR in copy 
        b       al_ret         		# then get out

########################################################################
#
#  l_str_cmp
#
#       Load String and Compare Byte (lscbx, lscbx.)
#               r25 = save area addr
#               r26 = DSISR --> becomes word_count
#               r27 = DAR       
#               r28 = MSR:DR=0 --> becomes scratch
#               r29 = MSR:DR=1
#               r30 = byte_count --> becomes scratch
#               r31 = scratch --> assemble data
########################################################################
l_str_cmp:
	st	r27,SAVE_WORK2(r25)	# save DAR for emulation code
        l       r31,SAVE_SRR0(r25)	# load SRR0 address from save area
        mtmsr   r29                     # turn on DR
	isync				# make sure DR is noticed
        l       r31,0(r31)              # load original instruction
        mtmsr   r28                     # turn off DR
	isync				# make sure DR is noticed
	st	r31,SAVE_WORK1(r25)	# save original instruction to scratch
        rlinm   r31,r31,24,0xF8		# pull out RB from I:16-20 (RB * 8)
        stb     r31,SAVE_RB(r25)   	# save RB
        l       r31,SAVE_XER(r25)       # load XER from save area
        andil.  r30,r31,0x7F            # mask off to get byte count
        cmpi    0,r30,0                 # if byte count <= 0
        ble     get_out    		# then go to get_out
        rlinm   r28,r31,24,0xFF		# get match byte
        stb     r28,SAVE_MATCH(r25)     # save match byte 
        ai      r27,r27,-1              # prepare DAR for update loads
        a       r30,r30,r27             # get address of last load
        rlinm   r28,r26,30,0xF8		# r28 = bits(6-10) of opcode, RT*8
        rlinm   r26,r26,3,0xF8		# r26 = bits(11-15) of opcode, RA*8
        stb     r28,SAVE_RT(r25)   	# save copy of RT
        stb     r26,SAVE_RA(r25)   	# save copy of RA
next:
        lbz     r26,SAVE_MATCH(r25)     # get match byte
        mtmsr   r29                     # turn on DR
	isync				# make sure DR is noticed
        xoril   r29,r29,MSR_DR          # toggle DR in copy 
        lbzu    r28,1(r27)              # load byte
        rlinm   r31,r28,24,0xFF000000	# insert into high position     
        cmp     0,r28,r26               # compare against match byte 
        beq     match
        cmp     0,r30,r27               # if we've done our last load
        beq     no_match        	# then go to no_match
        lbzu    r28,1(r27)              # load byte
        rlimi   r31,r28,16,0xFF0000	# insert into middle high position      
        cmp     0,r28,r26               # compare against match byte 
        beq     match
        cmp     0,r30,r27               # if we've done our last load
        beq     no_match        	# then go to no_match
        lbzu    r28,1(r27)              # load byte
        rlimi   r31,r28,8,0xFF00	# insert into middle low position       
        cmp     0,r28,r26               # compare against match byte 
        beq     match
        cmp     0,r30,r27               # if we've done our last load
        beq     no_match        	# then go to no_match
        lbzu    r28,1(r27)              # load byte
        or      r31,r31,r28             # insert into low position      
        cmp     0,r28,r26               # compare against match byte 
        beq     match
        cmp     0,r30,r27               # if we've done our last load
        beq     no_match        	# then go to no_match
        mtmsr   r29                     # turn off DR
	isync				# make sure DR is noticed
        xoril   r29,r29,MSR_DR          # toggle DR in copy 
        bl     	ld_reg         		# go load this register
        b      	next           		# go do next register

match:
        mtmsr   r29                     # turn off DR
	isync				# make sure DR is noticed
        xoril   r29,r29,MSR_DR          # toggle DR in copy 
        bl     	ld_reg         		# load the last register
	l	r31,SAVE_WORK2(r25)	# get DAR from save area
        sf      r31,r31,r27             # compute # of bytes done
        ai      r31,r31,1               # add 1
        l       r30,SAVE_XER(r25)  	# load XER from save area
        rlimi   r30,r31,0,0x7F		# insert done count
        st      r30,SAVE_XER(r25) 	# store XER back to save area
        l       r28,SAVE_WORK1(r25)     # load original instruction
        mtcr    r28                     # move original instruction to CR
        bc      false,31,al_ret         # leave if its not RC=1 form
        l       r31,SAVE_CR(r25)	# load CR from save area
        rlinm   r31,r31,0,0x0FFFFFFF	# mask off CR0
	rlimi	r31,r30,29,0x10000000   # insert the correct SO bit from XER
        oriu    r31,r31,0x2000          # set EQ bit of CR0
        st      r31,SAVE_CR(r25)	# save CR back to save area
        b      	al_ret			# get out

no_match:       
        # we enter here with DR on

        mtmsr   r29                     # turn off DR
	isync				# make sure DR is noticed
        xoril   r29,r29,MSR_DR          # toggle DR in copy 
        bl     	ld_reg         		# load the last register
get_out:
        l       r28,SAVE_WORK1(r25)     # load original instruction
        mtcr    r28                     # move original instruction to CR
        bc      false,31,al_ret         # leave if its not RC=1 form
        l       r30,SAVE_CR(r25)	# load CR from save area
        l       r31,SAVE_XER(r25)  	# load XER from save area
        rlinm   r30,r30,0,0x0FFFFFFF	# mask off CR0
	rlimi	r30,r31,29,0x10000000   # insert the correct SO bit from XER
        st      r30,SAVE_CR(r25)	# save CR back to save area
        b      	al_ret         		# get out

ld_reg:
        lbz     r28,SAVE_RT(r25)	# load RT 
        lbz     r26,SAVE_RA(r25)	# load RA
        cmpli   cr1,r26,0               # if RA == 0
        cmp     0,r26,r28               # if RA == RT
	beq	cr1, ld_RA_0		# then OK to load
        beq     skipx           	# then skip this load
ld_RA_0:
        lbz     r26,SAVE_RB(r25)	# load RB
        cmp     0,r26,r28               # if RB == RT
        beq     skipx           	# then skip this load
       .using   low,r26                 #
        cal     r26,loadtab(r28)        #
       .drop    r26                     #
        mflr    r28			# save LR
        mtlr    r26                  	# LR = branch addr in loadtab
        brl    				# branch into load table
        mtlr    r28                  	# restore LR
skipx:
        lbz     r28,SAVE_RT(r25) 	# load RT 
        ai      r28,r28,(1*8)           # increment RT
        cmpi    0,r28,(31*8)            # if RT <= 31 
        ble     save_rtx  		# then go to save_rt
        ai      r28,r28,-32*8           # else perform mod-32
save_rtx:
        stb     r28,SAVE_RT(r25) 	# save incremented copy
        br     				# return


########################################################################
#
#  cache_op
#
#       Handles dcbz, zeroes cache line. 
#               r25 = save area addr
#               r26 = DSISR --> cache block size
#               r27 = DAR       
#               r28 = MSR:DR=0 
#               r29 = MSR:DR=1
#               r30 = number of stores
#               r31 = scratch
#
########################################################################
cache_op:
        l       r26,syscfg_dcb(0)       # get cache block size from system
					# configuration structure
        ai      r31,r26,-1              # subtract 1 from cache size
        nor     r31,r31,r31             # 1's complement
        and     r27,r27,r31             # round DAR down to cache boundary
        ai      r27,r27,-4              # prepare for update stores
        a       r30,r27,r26             # r30 = address of last store
        xor     r31,r31,r31             # zero scratch register
        mtmsr   r29                     # turn on DR
	isync				# make sure DR is noticed
c_loop:
        stu     r31,4(r27)              # store 0's 
        cmp     0,r30,r27               # if last store
        bne     c_loop 			# go back to c_loop
        mtmsr   r28                     # else turn off DR
	isync				# make sure DR is noticed
        b      	al_ret         		# get out
        
        

########################################################################
#
#  algebra
#
#       extend the sign bit for algebraic forms
#
#       r31 = data
########################################################################
algebra:
        exts    r31,r31                 # extend bit 16 to the left
        br     				# return

########################################################################
#
#  reverse
#
#       reverse the order of the  4 bytes in r31
#
#       r31 = data
#       r25 = save area addr
#       r30 = scratch
########################################################################
reverse_half:
        rlinm   r31,r31,16,0xFFFF0000	# rotate high and low
reverse:
        st      r31,SAVE_WORK0(r25)
        lbrx    r31,r0, r25		# byte reverse 4 bytes
        br     				# return

########################################################################
#
#  update
#
#       Check if we should update.  If so, move DAR to RA
#
#       r27 = DAR+updates
#       r26 = DSISR
#       r30 = scratch
########################################################################
update:
        bc      false,UPD,al_ret        # return if no update required
        rlinm   r30,r26,3,0xF8		# r30 = bits(11-15) of opcode
                                        # = RA field multiplied by 8
        mfspr   r27,DAR                 # reload dar into 27(updates ruined it)
       .using   low,r30                 #
        cal     r30,updtab(r30)         # r30 = branch addr
       .drop    r30                     #
        mtlr    r30                  	# move it to link
        br     				# jump into update table

########################################################################
#
#  al_ret
#
#       Restore saved state and return.
#
########################################################################
al_ret:
        lm      r27, SAVE_SRR0(r25) 	# get SRR0,SRR1,LR,CR,XER from savearea
        cal     r27,4(r27)              # increment iar by 4
        mtspr   SRR0,r27                # restore SRR0 
        mtspr   SRR1,r28                # restore SRR1 
        mtlr   	r29                  	# restore LR 
        mtcr    r30                     # restore CR
        mtxer   r31                 	# restore XER
        lm      r26, SAVE_R26(r25)	# restore r26-r31
        l       r25, SAVE_R25(r25)	# restore r25
        rfi                             #

########################################################################
#
#  pr_flih_ppc
#
#	Power emulation entry point: perform a partial state save
#	in the common save ppda area used by the al_flih handler then
#	load the faulting instruction. This can page fault in MP
#	then the code must between alignlo and alignhi.
#
########################################################################

	include(pr_flih_ppc.m4)

alignhi:                                # marks end of page faulting 
                                        # alignment code
        .align 2

# include(i_machine.m4)
# include(except.m4)


