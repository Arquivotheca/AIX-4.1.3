# @(#)39        1.3  src/bos/kernel/ml/POWER/clock_ppc.s, sysml, bos411, 9430C411a 7/12/94 11:47:04
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: curtime_ppc, update_decrementer_ppc
#
# ORIGINS: 83
#
#
# LEVEL 1,  5 Years Bull Confidential Information
#
# 
#****************************************************************************
#
#  NAME:  curtime_ppc
#
#  FUNCTION:  Read the current time from the Time Base & converts it to
#		RTC format.
#
# 	curtime_ppc(timerstruc)		rc = timerstruc with current time in it
#
#  INPUT STATE:
#	r3 = ptr to the timerstruc into which is placed the current time
#
#  OUTPUT STATE:
#	The timerstruc pointed to by the pointer passed to this procedure
#	contains the current time as maintained by the realtime clock.
#
#  EXECUTION ENVIRONMENT:
# 	Supervisor state  : Yes
#	Manipulation of ppda items must be done disabled.
#
#*****************************************************************************
#	read TB_u, TB_l 
#	if ((TB_u == TB_ref_u) && ((TB_l & Xmask) == TB_ref_l)) {
#		upper_done:
#		lower = TB_l & ~Xmask;
#		if (Xfrac != 1)
#			lower /= Xfrac;
#		lower *= Xint;
#		while ( lower > billion ) {
#			secs += 1;
#			lower -= billion;
#		}
#		secs += secs_ref;
#		lower += nsecs_ref;
#		while ( lower > billion ) {
#			secs += 1;
#			lower -= billion;
#		}
#		return( secs, lower );
#	} else {
#		conversion on 54 bits, of (TB_u, TB_l & ~Xmask)
#		division by billion uses a table of precomputed values
#		see details below ...
#		update TB_ref_u, TB_ref_l, secs_ref, nsecs_ref
#		goto upper_done
#	}

	.file	"clock_ppc.s"

include(macros.m4)
include(systemcfg.m4)
include(machine.m4)

	S_PROLOG(curtime_ppc)

	.machine "ppc"
loop:
	mftbu	r4			# load TB upper
	mftb	r5			# load TB lower
	mftbu	r6			# load TB upper
	cmpw	r6, r4			# old = new ?
	bne	loop

	LTOC(r6, tb_Xmask, data)

	lwz	r12, syscfg_Xint(0)	# r12 = Xint
	lwz	r9,  syscfg_Xfrac(0)	# r9 = Xfrac
	lwz	r11, 0(r6)		# r11 = mask

	mfspr	r6, SPRG0		# get ppda pointer

	mfmsr	r0			# get current msr
	rlinm   r8, r0, 0, ~MSR_EE      # generate disabled msr
        mtmsr   r8                      # disable interrupts

	lwz	r7, ppda_TB_ref_u(r6)	# load TB_ref_u
	lwz	r8, ppda_TB_ref_l(r6)	# load TB_ref_l

	and	r10, r5, r11		# r10 = TB_l & Xmask
	cmp	cr0, r4, r7		# cr0 = (TBU == TB_ref_u)
	cmp	cr7, r10, r8		# cr7 = ((TB_l & Xmask) == TB_ref_l))
	cmpi	cr6, r9, 1		# cr6 = (Xfrac == 1)		
	crand	cr0*4+eq, cr0*4+eq, cr7*4+eq	# cr0 = ((TB_u == TB_ref_u) &&
						# ((TB_l & Xmask) == TB_ref_l))
	andc	r5,  r5, r11		# r5 = lower = TB_l & ~Xmask;
	bne	cr0, full_conv		# ((TB_u != TB_ref_u) ||
					#	 ((TB_l & Xmask) != TB_ref_l))


# TB upper is same as before, we know the corresponding (sec, nsec)
# Now convert TB lower bits

upper_done:

	lwz	r8,  ppda_nsec_ref(r6)	# r8 = nsecs_ref
	beq	cr6, nodiv		# if (Xfrac == 1) avoid division
					# (27% improvement)
# divide first to avoid overflow - drop remainder
	divwu	r5,  r5, r9		# lower = lower / Xfrac
nodiv:
# load constant 1000000000
	liu	r11, 1000000000 > 16	
	ori	r11, r11, 1000000000 & 0xffff	# r11 = 1000000000

# multiply by Xint - we know that the result fits in 32 bits
# ( Xmask has been chosen for that )
	mullw	r10, r5, r12		# r10 = lower = lower * Xint

	lwz	r7,  ppda_sec_ref(r6)	# r7 =  secs

	mtmsr	r0			# restore msr

# convert overflowing nsecs into secs
again:	cmpl	cr0, r10, r11		 
	blt	cumul			# branch if ( lower < billion )
	subfc	r10, r11, r10		# lower -= billion
	addi	r7, r7, 1		# secs += 1
	b	again

# add into nsecs
cumul:	add	r8, r8, r10		# r8 = lower += nsecs_ref

# May be still one overflowing second
	cmpl	cr0, r8, r11
	blt	exit			# branch if ( lower < billion )
	subfc	r8, r11, r8		# lower -= billion
	addi	r7, r7, 1		# secs += 1

exit:
	stw	r7, 0(r3)		# tv_sec = secs
	stw	r8, 4(r3)		# tv_nsec = lower
	
	br

full_conv:
# Convert upper bits (with low bits set to 0)
# input: r4 : TB_u				(reuse)
#        r10: TB_l with low bits reset		(reuse)
#	 r12: Xint
#	 r9 : Xfrac
#	 cr6: Xfrac==1
# preserve r5 (low bits), r6 (ppda), r3 (tod), r0 (msr)
# scratch: r7, r8, r11

# Store TB_u and TB_l for next call
	stw	r4,  ppda_TB_ref_u(r6)	# TB_ref_u = TB_u
	stw	r10, ppda_TB_ref_l(r6)	# TB_ref_l = TB_l & Xmask

# 1) divide by Xfrac - Xfrac is 0x0000xxxx
# Q1, R1  = TB_u / Xfrac
# r7  r4   r4     r9
	divwu	r7, r4, r9		# r7 = Q1 = TB_u / Xfrac
	mullw	r8, r7, r9
	subf	r4, r8, r4		# r4 = R1 = TB_u % Xfrac
# D1 = R1 << 16 + TB_l >> 16
# r8    r4        r10
	rlwinm	r8, r10, 16, 0x0000FFFF
	rlwimi	r8, r4, 16, 0xFFFF0000  # r8 = D1 = R1 << 16 + TB_l >> 16 

# Q2 , R2  = D1 / Xfrac
# r11  r4    r8     r9
	divwu	r11, r8, r9		# r11 = Q2 = D1 / Xfrac
	mullw	r4, r11, r9
	subf	r4, r4, r8		# r4 = R2 = D1 % Xfrac

# D2 = R2 << 16 + (TB_l & 0xFFFF)
# r10  r4          r10			TB_l is overwritten
	rlwimi	r10, r4, 16, 0xFFFF0000	# r10 = D2 = R2 << 16 + (TB_l & 0xFFFF)

# Q3 = D2 / Xfrac
# r4   r10   r9
	divwu	r4, r10, r9		# r4 = Q3 = D2 / Xfrac
# K_u = Q1 (r7)
# K_l = Q2 | Q3
#  r4   r11  r4
	rlwimi.	r4, r11, 16, 0xFFFF0000	# r4 = K_l = (Q2 << 16) + Q3

# 2) Multiply by Xint
#####################

# P1 = K_l * Xint -> P1_u + P1_l
#	r4   r12     r10     r8
	mulhwu	r10, r4, r12		# r10 = P1_u = (K_l * Xint) / 2^32
	mullw	r8, r4, r12		# r8 = P1_l = (K_l * Xint) % 2^32

# P2 = K_u * Xint -> P2_u + P2_l
#	r7    r12  dropped   r4
	mullw	r4, r7, r12		# r4 = P2_l = K_u * Xint

# NS_u = P1_u + P2_l
#  r10    r10     r4
# NS_l = P1_l
#  r8     r8
	add	r10, r10, r4		# r10 = NS_u = P1_u + P2_l

######################
# 3) S = NS/1000000000
######################

# input: r10: NS_u
#        r8 : NS_l
#
# sec = ((NS_u * 2^32) + Ns_l) / 1000000000
#
# 1000000000 = 0x100 * 0x3B9ACA
#
# First we divide by 0x100.
# Then division of high order bits by 0x3B9ACA uses a division table.
# Then low order bits are divided by 0x3B9ACA.
# c_tbl contains the quotients & remainders of the divisions:
#	 2^i / 0x3B9ACA

# Rotate right 8 bits, so that interesting bits are in lower
# D_u = NS_u >> 8
#  r10  r10
	rlwinm	r10, r10, 24, 0xFFFFFFFF	# r10 = D_u = NS_u / 0x100

# divide NS_u[0,23] by 0x3B9ACA, using c_tbl. 
# initiate loop
# table_ptr - mask - sec - nsec
#     r9       r12    r7    r4
	lil	r7, 0			# r7 = sec = 0
	lil	r4, 0			# r4 = nsec = 0
	liu	r12, 0x20		# r12 = mask = 0x200000
	LTOC(r9, c_tbl, data)		# r9 = table_ptr

# scratch
#   r11
next_bit:
	and.	r11, r12, r10		# test bit i of NS_u
	beq	not_set			# branch if bit i not set
	lwz	r11, 0(r9)		# r11 = 2^i / 0x3B9ACA
	add	r7, r7, r11		# sec += r11
	lwz	r11, 4(r9)		# r11 = 2^i % 0x3B9ACA
	add	r4, r4, r11		# nsec += r11
not_set:
	rlwinm.	r12, r12, 31, 0x7FFFFFFF # mask = mask >> 1
	addi	r9, r9, 8		# r9 = table_ptr++
	bne	next_bit		# last bit ?

# divide NS_u[24,31] | NS_l[0,23] by 0x3B9ACA
# D_l = NS_u[24,31] | NS_l[0,23]
#  r10    r10          r8
	rlwimi	r10, r8, 24, 0x00FFFFFF	# r10 = D_l = NS_u[24,31] | NS_l[0,23]

# 0x3B9ACA (const) 
#    r11
	liu	r11, 0x3C
	addi	r11, r11, 0xFFFF9ACA	# r11 = 0x3B9ACA

# Compute lower division
# Q  , R    =  D_l / 0x3B9ACA
# r12  r12       r10     r11
	divwu	r12, r10, r11		# r12 = Q = D_l / 0x3B9ACA
	add	r7, r7, r12		# sec += Q
	mullw	r12, r12, r11
	subf	r12, r12, r10		# r12 = R = D_l % 0x3B9ACA
	add	r4, r4, r12		# nsec += R

# Convert overflowing nsecs in sec , nsec
# Q  , nsec =  nsec / 0x3B9ACA
# r12   r4      r4      r11
	divwu	r12, r4, r11		# r12 = Q = nsec / 0x3B9ACA
	add	r7, r7, r12		# sec += Q
	mullw	r12, r12, r11
	subf	r4, r12, r4		# nsec = nsec % 0x3B9ACA

# Shift nsec and insert in original lower word, keeping low bits
	rlwimi	r8, r4, 8, 0xFFFFFF00	# r8 = nsec = nsec * 0x100 + NS_l[24,31]

# Store sec, nsec for next call
	stw	r7, ppda_sec_ref(r6)	# store secs
	stw	r8, ppda_nsec_ref(r6)	# store nsecs

# Now compute on low bits, using above method

	lwz	r9,  syscfg_Xfrac(0)	# load Xfrac
	lwz	r12, syscfg_Xint(0)	# load Xint

	b	upper_done

#****************************************************************************
#
#  NAME:  update_decrementer_ppc
#
#  FUNCTION:  Convert nanoseconds to Time Base ticks & write to PowerPC 
#		decrementer.
#
# 	update_decrementer_ppc(nanos)
#
#  INPUT STATE:
#	r3 = number of nanoseconds
#
#  OUTPUT STATE:
#	Decrementer updated
#
#  EXECUTION ENVIRONMENT:
# 	Supervisor state  : Yes
#
#*****************************************************************************

	S_PROLOG(update_decrementer_ppc)

	lwz	r12, syscfg_Xint(0)	# load Xint
	lwz	r9, syscfg_Xfrac(0)	# load Xfrac
	add	r3, r3, r12
	subi	r3,  r3, 1 	# r3 = ns + Xint - 1 
	divwu	r3, r3, r12	# r3 = r3/Xint
	mullw	r3, r3, r9	# r3 = r3 * Xfrac

	mtspr	MT_DEC, r3	# set the dec
	br
	FCNDES(update_decrementer_ppc)

	.csect	curtime_ppc_tbl[RW]

DATA(tb_Xmask):
	.long	0		# mask for insulating
	.globl	DATA(tb_Xmask)


# Precomputed division table used by curtime_ppc() to compute 2^i / 0x3B9ACA.
#	1st word: Q = 2^i / 0x3B9ACA
#	2nd word: R = 2^i % 0x3B9ACA
#       53 >= i >= 32

DATA(c_tbl):
	.long	0x89705F41	# 0x20000000000000 / 3B9ACA
	.long	0xCBCB6		# 0x20000000000000 % 3B9ACA
	.long	0x44B82FA0
	.long	0x242BC0
	.long	0x225C17D0
	.long	0x1215E0
	.long	0x112E0BE8
	.long	0x90AF0
	.long	0x89705F4
	.long	0x48578
	.long	0x44B82FA
	.long	0x242BC
	.long	0x225C17D
	.long	0x1215E
	.long	0x112E0BE
	.long	0x1E5E14
	.long	0x89705F
	.long	0xF2F0A
	.long	0x44B82F
	.long	0x2564EA
	.long	0x225C17
	.long	0x307FDA
	.long	0x112E0B
	.long	0x360D52
	.long	0x89705
	.long	0x38D40E
	.long	0x44B82
	.long	0x3A376C
	.long	0x225C1
	.long	0x1D1BB6
	.long	0x112E0
	.long	0x2C5B40
	.long	0x8970
	.long	0x162DA0
	.long	0x44B8
	.long	0xB16D0
	.long	0x225C
	.long	0x58B68
	.long	0x112E
	.long	0x2C5B4
	.long	0x897
	.long	0x162DA
	.long	0x44B		# 100000000 / 3B9ACA
	.long	0x1E7ED2	# 100000000 % 3B9ACA

	.globl	DATA(c_tbl)

	.toc
	TOCE(tb_Xmask, data)
	TOCE(c_tbl, data)

include(param.m4)
include(scrs.m4)
include(low_dsect.m4)
