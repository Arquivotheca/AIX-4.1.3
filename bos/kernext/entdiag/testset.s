# @(#)99	1.1.1.3  src/bos/kernext/entdiag/testset.s, diagddent, bos411, 9428A410j 1/13/94 15:45:44
#*****************************************************************************
#
# COMPONENT_NAME: (SYSXENT) 
#
# FUNCTIONS: test_set cache_inval
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1990
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

	.file "testset.s"

#------------------------------------------------------------------------------
#
# NAME: test_set
#
# FUNCTION: perform an atomic test and set function
#	if (!*t0 && *t1 == *t2)
#	{
#		*s = 0;
#		return(0);
#	}
#	else
#	{
#		return(-1);
#	}
#
# CALL:	int test_set(t0, t1, t2, s)
#	int *t0;
#	int *t1;		/* value 1 to test */
#	int *t2;		/* value 2 to test */
#	int *s;			/* word to set */
#
# EXECUTION ENVIORNMENT:
#	Called from the offlevel interrupt handler
#
# RETURNS:
#	0 - if values were equal and set opperation was done
#	-1 - if values were not eaual and set opperation was not done
#
#------------------------------------------------------------------------------

	S_PROLOG(test_set)
	mfmsr	r7			# get current msr
	rlinm	r0, r7, 0, ~MSR_EE	# generate disabled msr
	mtmsr	r0			# disable interrups
	l	r3, 0(r3)		# get value 0 to compare
	l	r8, 0(r4)		# get value 1 to compare
	l	r9, 0(r5)		# get value 2 to compare
	cmpi	cr1, r3, 0		# *t0 == 0?
	cmp	cr0, r8, r9		# check if the two are equal
	bne	cr1, no_set		# branch if *t0 != 0
	lil	r3, 0
	bne	cr0, no_set		# branch if values are not equal
	st	r3, 0(r6)		# do set opperation
	mtmsr	r7			# enable interrupts
	br				# return
no_set:
	cal	r3, -1(0)		# set bad retrun code
	mtmsr	r7			# enable interrupts
	br

#
# get_pending -	atomically get and clear the pending interrupts
#
# Input:
#	pp	-	^ to pending interrupts
#
# Returns:
#	*pp, and *pp set to 0
#

	S_PROLOG(get_pending)

	mfmsr	r5			# get current msr
	rlinm	r0, r5, 0, ~MSR_EE	# generate disabled msr
	mtmsr	r0			# disable interrups

	mr	r4, r3			# copy r3 to r4
	l	r3, 0(r4)		# load up *pp

	lil	r0, 0			# r0 <- 0
	st	r0, 0(r4)		# *pp <- 0

	mtmsr	r5			# enable interrupts
	br


include(INCLML/machine.m4)

