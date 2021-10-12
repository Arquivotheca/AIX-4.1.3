static char sccsid[] = "@(#)46	1.8  src/bos/kernel/proc/POWER/exception.c, sysproc, bos411, 9428A410j 7/27/93 18:51:58";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: exception
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*#print(off)*/
#include <sys/types.h>
#include <sys/reg.h>			/* Register definitions. */
#include <sys/mstsave.h>		/* Machine state save area */
/*#print(on)*/

/* 
 *  Name:      exception -- kernel exception slih
 *
 *  Function:
 *         -- set up mstsave to be dispatched at longjmpx()
 *
 */

extern int longjmpx();
extern g_toc;

struct f_ptr 
{
	char *entry_point;
	char *toc_addr;
};

static struct f_ptr *long_ep = (struct f_ptr *)longjmpx;

void
exception(struct mstsave *mst, int rc)
{

	/* Before mucking with the iar, toc, and arg1 regs, save them */
	/* so that if we crash and burn , we know where we were */
	mst->o_iar = mst->iar;
	mst->o_toc = mst->gpr[TOC];
	mst->o_arg1 = mst->gpr[ARG1];

	/* is there an exception-branch address ?
	 */
	if (mst->excbranch)
	{
		mst->iar = mst->excbranch;
		mst->excbranch = 0;
		mst->gpr[ARG1] = (ulong_t)rc;
		return;
	}

	/* Store the iar of the entry point of longjmpx in the mstsave area */
	mst->iar = (ulong_t)(long_ep->entry_point);
	mst->gpr[TOC] = (ulong_t)g_toc;
	mst->gpr[ARG1] = (ulong_t)rc;
	return;
}
