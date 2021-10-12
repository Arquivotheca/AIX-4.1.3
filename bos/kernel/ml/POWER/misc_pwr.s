# @(#)42	1.6  src/bos/kernel/ml/POWER/misc_pwr.s, sysml, bos411, 9428A410j 5/23/94 19:06:47
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: misc. functions
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1992, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

	.file	"misc_pwr.s"
	.machine "pwr"


#
# Move to eis
#
# input:	r3 = value to write to eis0
# 		r4 = value to write to eis1
#
# output:	eis updated
#
	S_PROLOG(mteis)
	mfsr	r5, sr15		# save contents of sr15
	cau	r6, r0, 0x8000		# value for i/o seg reg
	mtsr	sr15, r6		# put in sr15
	cau	r6, 0, 0xf000		# set r6 to access i/o segment
	oril	r6, r6, EIS0		# add offset of EIS
	stsi	r3, r6, 8		# store new values
	mtsr	sr15, r5		# restore sr15
	br

#
# Move from eim
#
# input:        r3 = ptr to eim0
#               r4 = ptr to eim1
#
# output:       *r3=eim0,*r4=eim1 value
#
	S_PROLOG(mfeim)
	mfsr	r5,r15
	cau	r6,r0,0x8000		#value for i/o seg reg
	mtsr	15,r6			# put i/o value in sr15
	cau	r6,r0,0xf000		# set r6 to access i/o segment
	mr      r7,r30                  # save r30 and r31
	mr      r8,r31
	l	r30,EIM0(r6)
	l       r31,EIM1(r6)            # get EIM0 and EIM1
	st      r30,0(r3)               # store eim0
	st      r31,0(r4)               # store eim1
	mr      r30,r7                  # restore r30 and r31
	mr      r31,r8
	mtsr	15,r5			# restore register 31
	br
	FCNDES(mfeim)

#
# Move to eim
#
# input:	r3 = value to write to eim0
# 		r4 = value to write to eim1
#
# output:	eim updated
#
	S_PROLOG(mteim)
	mfsr	r5,15			# save sr15
	cau	r6,r0,0x8000		#value for i/o seg reg
	mtsr	15,r6			# to seg reg 15 for i/o
	cau	r6,r0,0xf000		# set r6 to access i/o segment
	mr      r7,r30                  # save r30 and r31
	mr      r8,r31
	mr      r30,r3
	mr      r31,r4
	st      r30,EIM0(r6)            # store EIM0 and EIM1
	st	r31,EIM1(r6)
	mr      r30,r7                  # restore r30 and r31
	mr      r31,r8
	mtsr	15,r5			# restore sr15
	br
	FCNDES(mteim)

#------------------------------------------------------------------------------
#
# FUNCTION: read eis registers
#
# CALL:	void mfeis(eis0, eis1)
#	int *eis0;		/* return value of eis0 here */
#	int *eis1;		/* return value of eis1 here */
#
# EXECUTION ENVIRONMENT:
#	Interrupt level, and process level
#
# RETURNS: None
#
#------------------------------------------------------------------------------

	S_PROLOG(mfeis)
	cau	r6, 0, BUID0		# value for i/o seg reg
	mfsr	r5, sr15		# save seg reg 15
	mtsr	sr15, r6		# to seg reg 15 for i/o
	cau	r6, 0, 0xf000		# set r6 to access i/o segment
	cal	r6, EIS0(r6)		# add offset to EISs
	lsi	r7, r6, 8		# load EIM0 and EIS1
	stsi	r7, r6, 8		# restore value of EISs
	mtsr	sr15, r5		# restore seg reg 15
	st	r7, 0(r3)		# set EIS0
	st	r8, 0(r4)		# set EIS1 (should be 0)
	br
	FCNDES(mfeis)

#
#
# FUNCTION: read peis registers
#
# CALL: void mfpeis(peis0, peis1)
#       int *peis0;             /* return value of peis0 here */
#       int *peis1;             /* reture value of peis1 here */
#
# EXECUTION ENVIORNMET:
#       Interrupt level, and process level
#
# RETURNS: None
#
#

        S_PROLOG(mfpeis)
        cau     r6, 0, BUID0            # value for i/o seg reg
        mfsr    r5, sr15                # save seg reg 15
        mtsr    sr15, r6                # to seg reg 15 for i/o
        cau     r6, 0, 0xf000           # set r6 to access i/o segment
        cal     r6, PEIS0(r6)           # add offset to PEISs
        lsi     r7, r6, 8               # load PEIS0 and PEIS1
        mtsr    sr15, r5                # restore seg reg 15
        st      r7, 0(r3)               # set PEIS0
        st      r8, 0(r4)               # set PEIS1
        br
        FCNDES(mfpeis)

#
#
# FUNCTION: Set bit 63 in EIS (i.e. bit 31 in EIS1) for RS1
#
# CALL: void i_eis_set(void)
#
# ACCESSED: through branch table entry i_soft
#
# RETURNS:
#       None
#
#

        S_PROLOG(i_eis_set)
	cal	r5, INT_OFFLVLWD(0)	#
        cau     r11, 0, BUID0           # calculate the IO segment value
        mfsr    r12, sr15               # save seg reg 15
        cau     r4, 0, 0xF000           # calculate effective addresss
        mtsr    sr15, r11               # load IO seg reg
        st      r5, EIS1(r4)            # Set bit in EIS1
        mtsr    sr15, r12               # restore segment register
        br
        FCNDES(i_eis_set)
#
#
# FUNCTION: Reset bit 63 in EIS ( i.e. bit 31 in EIS1) for RS1
#
# CALL: void i_eis_reset(void)
#
# ACCESSED: through branch table entry i_soft_reset
#
# EXECUTION ENVIORNMET:
#       Interrupt level, and process level
#
# RETURNS: None
#
#
        S_PROLOG(i_eis_reset)
        cau     r6, 0, BUID0            # value for i/o seg reg
        mfsr    r5, sr15                # save seg reg 15
        mtsr    sr15, r6                # to seg reg 15 for i/o
        cau     r6, 0, 0xF000           # set r6 to access i/o segment
        cal     r6, EIS0(r6)            # add offset to EISs
        lsi     r7, r6, 8               # load EIS0 and EIS1
	rlinm	r8, r8, 0, ~INT_OFFLVLWD #
        stsi    r7, r6, 8               # restore value of EISs
        mtsr    sr15, r5                # restore seg reg 15
        br
        FCNDES(i_eis_reset)

#
#
# FUNCTION: Set bit in PEIS0/1 for RS2
#
# CALL: void i_peis_set(void)
#
# ACCESSED: through branch table entry i_soft
#
# RETURNS:
#       None
#
#

        S_PROLOG(i_peis_set)
        cal     r3, INT_OFFLVL(0)	#
        cau     r3, r3, ICO_SLI         # add in ICO cmd to set level
        mtspr   ILCR, r3                # set the level
        br
        FCNDES(i_peis_set)

#
#
# FUNCTION: Clear level in PEIS0/1 for RS2
#
# CALL: void i_peis_reset(void)
#
# ACCESSED: through branch table entry i_soft_reset
#
# RETURNS:
#       None
#
#

        S_PROLOG(i_peis_reset)
        cal     r3, INT_OFFLVL(0)	#
        cau     r3, r3, ICO_CLI         # add in ICO cmd to clear level
        mtspr   ILCR, r3                # clear the level
        br
        FCNDES(i_peis_reset)

#*******************************************************************************
#
# NAME: mfdec_pwr
#
# FUNCTION: read Power Decrementer
#
# RETURNS: decrementer value
#
#*******************************************************************************

	S_PROLOG(mfdec_pwr)
	mfspr	r3, MF_DEC_PWR
	br
	FCNDES(mfdec_pwr)

#*******************************************************************************
#
# NAME:
#    get_from_list_pwr
#
# FUNCTION:
#    This function provides an atomic means to remove the first
#    element from a singly linked list.
#
# INPUTS:
#    r3 -- address of list head (anchor)
#    r4 -- offset of the "next" pointer within element.
#
# RETURNS: 
#    The address of the element removed.
#
# NOTES:
#    -This function is designed as a lightweight means of removing an
#     element from the head of a singly-linked list.  The head (or anchor)
#     of this list is presumed to be allocated (i.e. _not_ simply a
#     pointer) and pinned.  When the next field of the head is nil, then
#     the list is empty.  The list is terminated by a nil next pointer.
#
#    -The strategy for accomplishing the list removal is as follows: First,
#     the list head must reside in pinned memory. Next, interrupts are
#     disabled to INTMAX, and the next pointer is loaded from the head into
#     a register and dereferenced.  Upon derefernce, it is possible that we
#     page fault (and hence, get put to sleep); therefore, after completing
#     the derefernce, we check the original next pointer we loaded into a
#     register with what is currently in the head's next pointer. If the
#     addresses are not equal, then we loop back to try again.  Otherwise,
#     we store the appropriate pointers to remove the element from the
#     list, drop interrupt level, and return. 
#
#    N.B.  This function depends of the low bytes of memory being zero.
#          Currently, the low 256 bytes of memory are zeroed; thus, the
#          offset passed in via r4 must be less than 256.
#    
#*******************************************************************************
	S_PROLOG(get_from_list_pwr)
	mfmsr	r9			# save MSR into r9
	rlinm	r5,r9,0,~MSR_EE		# turn off interrupt bit
	mtmsr	r5			# turn off interrupts
retry_get:
	lx	r5,r4,r3		# load head->next
	lx	r6,r4,r5		# dereference it
					# may page fault and loose
					# processor here.
	lx	r7,r4,r3		# get what's at head->next now
	cmp	cr0,r7,r5		# compare old and new
	bne	retry_get		# not equal, branch back and retry

	stx	r6,r4,r3		# store new head pointer
	mr	r3,r5			# set return value
	mtmsr	r9			# restore interrupt state
	br				# return
	FCNDES(get_from_list_pwr)

#*******************************************************************************
#
# NAME:
#    put_onto_list_pwr
#
# FUNCTION:o
#    This function provides an atomic means to add an element to the
#    head of a singly linked list.
#
# INPUTS:
#    r3 -- address of list head (anchor).
#    r4 -- address of new head of list.
#    r5 -- offset of the "next" pointer within element.
#
# RETURNS: 
#    None.
#
# NOTES:
#    -This function is designed as a lightweight means of adding an
#     element to the head of a singly-linked list.  The head (or anchor)
#     of this list is presumed to be allocated (i.e. _not_ simply a
#     pointer) and pinned.  When the next pointer in the head is nil, then
#     the list is empty.  The list is terminated by a nil next pointer.
#
#    -The strategy for accomplishing the list insertion is as follows:
#     First, the list head must reside in pinned memory. Next, interrupts
#     are disabled to INTMAX, and the next pointer is loaded from the head
#     into a register.  Now, we load the next pointer from the element
#     passed in. We don't really care what is at this location, we just
#     need to ensure that we have the page resident. This access may cause
#     a page fault, and, hence, we may get put to sleep; therefore, after
#     completing the load, we check the original next pointer we loaded
#     into a register with what is currently in the head's next pointer.
#     If the two are not equal, we loop back to try again.  Otherwise, we
#     proceed to store the appropriate pointers to add the element to the
#     list, drop interrupt level, and return.
#
#
#    N.B.  This function depends of the low bytes of memory being zero.
#          Currently, the low 256 bytes of memory are zeroed; thus, the
#          offset passed in via r5 must be less than 256.
#    
#*******************************************************************************
	S_PROLOG(put_onto_list_pwr)
	mfmsr	r9			# save MSR into r9
	rlinm	r6,r9,0,~MSR_EE		# turn off interrupt bit
	mtmsr	r6			# turn off interrupts
retry_put:
	lx	r6,r5,r3		# load head->next
	lx	r7,r5,r4		# dereference ele->next
					# may page fault and loose
					# processor here.
	lx	r8,r5,r3		# get what's at head->next now
	cmp	cr0,r6,r8		# compare old and new
	bne	retry_put		# not equal, branch back and retry

#
# At this point:
#    r3    -- address of list head
#    r4    -- address of new ele
#    r5    -- offset of next pointer
#    r6,r8 -- current head->next
#    r7    -- the old head->next (don't care what it is)
#    r9    -- the old MSR
#
	stx	r4,r5,r3		# make head->next = ele
	stx	r6,r5,r4		# make ele->next = old head->next
	mtmsr	r9			# restore interrupt state
	br				# return
	FCNDES(put_onto_list_pwr)


ifdef(`_POWER_MP',`

# The following functions are overlays and can be inlined, therefore their
# size is kept in overlay.m4 and overlay.h, and their register usage is kept
# in ppda.h

#*******************************************************************************
#
# NAME: my_ppda
#
# FUNCTION: gets ppda
#
# RETURNS: ppda address
#
#*******************************************************************************

	.csect	my_ppda_pwr_overlay[RO]
DATA(my_ppda_pwr):
	.globl	DATA(my_ppda_pwr)

	l	r3, proc_arr_addr(0)
	br

#*******************************************************************************
#
# NAME: my_csa
#
# FUNCTION: gets csa
#
# RETURNS: current mst address
#
# NOTE: !!! By ifdefing _SLICER the disabling part, we assume that RS machines
#  !!! are always UP.
#
#*******************************************************************************

	.csect	my_csa_pwr_overlay[RO]
DATA(my_csa_pwr):
	.globl	DATA(my_csa_pwr)

ifdef(`_SLICER',`
	mfmsr	r4
	rlinm	r3, r4, 0, ~MSR_EE
	mtmsr	r3
')
	l	r3, proc_arr_addr(0)
	l	r3, ppda_csa(r3)
ifdef(`_SLICER',`
	mtmsr	r4
')
	br


#*******************************************************************************
#
# NAME: get_curthread
#
# FUNCTION: gets curthread
#
# RETURNS: current thread structure address
#
# NOTE: !!! By ifdefing _SLICER the disabling part, we assume that RS machines
#  !!! are always UP.
#
#*******************************************************************************

	.csect	get_curthread_pwr_overlay[RO]
DATA(get_curthread_pwr):
	.globl	DATA(get_curthread_pwr)

ifdef(`_SLICER',`
	mfmsr	r4
	rlinm	r3, r4, 0, ~MSR_EE
	mtmsr	r3
')
	l	r3, proc_arr_addr(0)
	l	r3, ppda_curthread(r3)
ifdef(`_SLICER',`
	mtmsr	r4
')
	br


#*******************************************************************************
#
# NAME: set_csa
#
# FUNCTION: sets csa
#
# INPUTS: mst address to make current
#
# NOTES: Caller must ensure that pre-emption is disabled
#
#*******************************************************************************

	.csect	set_csa_pwr_overlay[RO]
DATA(set_csa_pwr):
	.globl	DATA(set_csa_pwr)

	l	r4, proc_arr_addr(0)
	st	r3, ppda_csa(r4)
	l	r4, ppda_mstack(r4)
	st	r3, mstprev(r4)
	br


#*******************************************************************************
#
# NAME: set_curthread
#
# FUNCTION: sets curthread
#
# INPUTS: thread structure address to make current
#
# NOTES: Caller must ensure that pre-emption is disabled
#
#*******************************************************************************

	.csect	set_curthread_pwr_overlay[RO]
DATA(set_curthread_pwr):
	.globl	DATA(set_curthread_pwr)

	l	r4, proc_arr_addr(0)
	st	r3, ppda_curthread(r4)
	br
')


include(scrs.m4)
include(machine.m4)
include(i_machine.m4)
include(low_dsect.m4)
include(mstsave.m4)
