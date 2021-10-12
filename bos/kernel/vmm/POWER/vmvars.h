/* @(#)61	1.5.1.3  src/bos/kernel/vmm/POWER/vmvars.h, sysvmm, bos411, 9436B411a 9/1/94 17:50:01 */
#ifndef	_h_VMVARS
#define	_h_VMVARS

/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 * tunable variables for VMM,
 * when adding fields to this struct remember
 * to add and initialize them in vmmisc.c
 */

struct vmvars {
	int	minfree;
	int	maxfree;
	int	minperm;
	int	maxperm;
	int	pfrsvdblks;
	int	npswarn;
	int	npskill;
	int	minpgahead;
	int	maxpgahead;
	int	maxpdtblks;
	int	numsched;
	int	htabscale;
	int	aptscale;
	int	pd_npages;
};

/* vmconfig contains configurable variables.
 */
struct vmconfig
{
	int	maxpout;	/* non-fblru pageout up limit - i/o pacing   */ 
	int	minpout;	/* non-fblru pageout down limit - i/o pacing */ 
};

#ifdef	_KERNSYS
extern struct vmvars vmvars;
#endif	/* _KERNSYS */

#endif	/* _h_VMVARS */
