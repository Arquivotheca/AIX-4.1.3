# @(#)48	1.10  src/bos/kernel/ml/POWER/macros.m4, sysml, bos41J, 9518A_all 5/2/95 08:53:20

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
# (C) COPYRIGHT International Business Machines Corp. 1992, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
 
#
# LEVEL 1,  5 Years Bull Confidential Information
#


#******************************************************************************
#
# NAME: SPECIFIC_CODE
#
# CALL: SPECIFIC_CODE(CR, SCRATCH, MP_RS_CODE, MP_PC_CODE, UP_CODE)
#
# FUNCTION: generates specific code (including run-time model check)
#
# NOTES:
#	The CR and SCRATCH are used for a run-time model specific check and
#	may be used in XX_CODE
#
#*******************************************************************************

define(`SPECIFIC_CODE_LABEL', 0)

define(SPECIFIC_CODE,`

	define(`SPECIFIC_CODE_LABEL', incr(SPECIFIC_CODE_LABEL))

	ifdef(`_POWER_MP',`
	ifdef(`_POWER_RS',`
	ifdef(`_POWER_PC',`
		l	$2, syscfg_arch(0)	# get machine architecture
		cmpi	$1, $2, POWER_PC	# check for PPC
		beq	$1, `PC_SPECIFIC_CODE'SPECIFIC_CODE_LABEL
	',)
`RS_SPECIFIC_CODE'SPECIFIC_CODE_LABEL:
		$3
	ifdef(`_POWER_PC',`
		b	`END_SPECIFIC_CODE'SPECIFIC_CODE_LABEL
	',)
	',)
	ifdef(`_POWER_PC',`
`PC_SPECIFIC_CODE'SPECIFIC_CODE_LABEL:
	ifdef(`_POWER_RS',`
		.machine "push"
		.machine "ppc"
	',)
		$4
	ifdef(`_POWER_RS',`
		.machine "pop"
	',)
	',)
`END_SPECIFIC_CODE'SPECIFIC_CODE_LABEL:
	',`
		$5
	')
')


ifdef(`_SLICER',`

# Slicer-only macros.  As in any real-MP system, access to ppda entries must
# be atomic.  For a normal RS/6000, there is only one cpu (one ppda) therefore,
# access need not be serialized. In the Slicer, however, there are multiple cpus
# and therefore multiple ppdas, access to which must be atomic.  Rather than
# complicate the already complex SPECIFIC_CODE macro for the Slicer, they have
# each been re-written for RS/6000 platforms (Slicer) only.

#
# See comment for GET_CSA below
#
define(GET_CSA,`
		mfmsr	$2
		rlinm	$3, $2, 0, ~MSR_EE
		mtmsr	$3
		l	$3, proc_arr_addr(0) 
		l	$3, ppda_csa($3)
		mtmsr	$2
')

#
# See comment for CSA_UPDATED below
#
define(CSA_UPDATED,`
')

#
# See comment for GET_CURTHREAD below
#
define(GET_CURTHREAD, `
		mfmsr	$2
		rlinm	$3, $2, 0, ~MSR_EE
		mtmsr	$3
		l	$3, proc_arr_addr(0) 
		l	$3, ppda_curthread($3)
		mtmsr	$2
')

#
# See comment for  GET_PPDA below
#
define(GET_PPDA, `
		l	$2, proc_arr_addr(0)
')

',` # _SLICER

#******************************************************************************
#
# NAME: GET_CSA
#
# CALL: GET_CSA(CR, SCRATCH, GPR)
#
# FUNCTION: loads GPR with the current value of csa
#
# NOTES:
# 	Do not use r0 for GPR
#
#	MP_RS code will only run on a UP_RS machine (one ppda only)
#
#*******************************************************************************

define(GET_CSA,`
	SPECIFIC_CODE($1, $2,
	  ` # MP_RS
		l	$3, proc_arr_addr(0)
		l	$3, ppda_csa($3)
	',` # MP_PC
		mfspr	$3, SPRG3
	',` # UP
		l	$3, csa(0)
	')
')


#******************************************************************************
#
# NAME: CSA_UPDATED
#
# CALL: CSA_UPDATED(CR, SCRATCH, GPR)
#
# FUNCTION: applies side-effects when ppda_csa has been updated directly
#
# NOTES:
#	GPR is an input parameter which must contain the new csa value
#
#	Currently, loading SPGR3, when MP_PC is set, is the only side-effect
#
#*******************************************************************************

define(CSA_UPDATED,`
	SPECIFIC_CODE($1, $2,
	  ` # MP_RS
	',` # MP_PC
		mtspr	SPRG3, $3
	',` # UP
	')
')


#*******************************************************************************
#
# NAME: GET_CURTHREAD
#
# CALL: GET_CURTHREAD(CR, SCRATCH, GPR)
#
# FUNCTION: loads GPR with the current value of curthread
#
# NOTES:
# 	Do not use r0 for GPR
#
#	MP_RS code will only run on a UP_RS machine (one ppda only)
#
#*******************************************************************************

define(GET_CURTHREAD,`
	SPECIFIC_CODE($1, $2,
	  ` # MP_RS
		l	$3, proc_arr_addr(0)
		l	$3, ppda_curthread($3)
	',` # MP_PC
		mfspr	$3, SPRG2
	',` # UP
		l	$3, curthread(0)
	')
')


#******************************************************************************
#
# NAME: GET_PPDA
#
# CALL:	GET_PPDA(CR, GPR)
#
# FUNCTION: loads GPR with ppda of current processor
#
# NOTES:
#	MP_RS code will only run on a UP_RS machine (one ppda only)
#
#*******************************************************************************

define(GET_PPDA,`
	SPECIFIC_CODE($1, $2,
	  ` # MP_RS
		l	$2, proc_arr_addr(0)
	',` # MP_PC
		mfspr	$2, SPRG0
	',` # UP
		cal	$2, ppda(0)
	')
')

') # _SLICER


#*******************************************************************************
#
# NAME: ATOMIC_INC
#
# CALL: ATOMIC_INC(CR, GPR1, GPR2)
#
# FUNCTION: atomically increments the word pointed to by GPR2.
#
# NOTES:
#	CR and GPR1 are destroyed; cr0 is destroyed too
#
#	Caller must be disabled
#
#*******************************************************************************

define(`ATOMIC_INC_LABEL', 0)

define(ATOMIC_INC,`

	define(`ATOMIC_INC_LABEL', incr(ATOMIC_INC_LABEL))

	SPECIFIC_CODE($1, $2,
	  ` # MP_RS
		lx	$2, 0, $3
		addi	$2, $2, 1
		stx	$2, 0, $3
	',` # MP_PC
`LOOP_ATOMIC_INC'ATOMIC_INC_LABEL:
		lwarx	$2, 0, $3
		addi	$2, $2, 1
		stwcx.	$2, 0, $3
		bne	cr0, `LOOP_ATOMIC_INC'ATOMIC_INC_LABEL
	',` # UP
		lx	$2, 0, $3
		addi	$2, $2, 1
		stx	$2, 0, $3
	')
')
