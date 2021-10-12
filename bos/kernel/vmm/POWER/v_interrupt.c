static char sccsid[] = "@(#)97	1.14  src/bos/kernel/vmm/POWER/v_interrupt.c, sysvmm, bos411, 9436D411a 9/8/94 04:11:02";
/*
 *   COMPONENT_NAME: SYSVMM
 *
 *   FUNCTIONS: getisidx
 *		v_interrupt
 *
 *   ORIGINS: 27,83
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include "vmsys.h"
#include <sys/user.h>
#include <sys/low.h>
#include <sys/errno.h>
#include <sys/inline.h>

/*
 * v_interrupt(p)
 *
 * removes a specified process from page wait provided
 * that the page wait is interruptable.
 *
 * interruptable waits are supported for interruptable
 * client segments only and consists of waits on a 
 * page, segment i/o level, and segment protection
 * message.
 *
 * p is a pointer to a process table entry.
 *
 * a check will be made to determine if the specified
 * process is waiting on an interruptable page wait.
 * if so, a call will be made to v_readyt() to ready
 * the process.  the program check handler will be
 * called with an exception value of EINTR if the 
 * process was running in supervisor mode at the time
 * of the page wait.
 *
 * this routine is called only by process management
 * signal delivery and executes at VMM interrupt level
 * on a fixed stack without back-tracking enabled.
 *
 * RETURN VALUE
 *
 *	NONE
 * 
 * SMP
 *	no serialization needed by this routine per se,
 * 	serialization is handled in the v_readyt() subroutine
 * 	which also checks that the thread is still on the wait
 *	list (under the proc_int_lock, which is used to serialize
 *	access to the list in addition to the thread itself).
 */

v_interrupt(t)
struct thread *t;
{
	int srsave,sidx;
	struct mstsave *mst;
	uint vaddr;

	/* check for VMM page wait.
	 */
	if (t->t_wtype != TWPGIN)
	{
		return(0);
	}

	/* get sidx.
	 */
	if ((sidx = getisidx(t->t_wchan2)) < 0)
	{
		return(0);
	}

	/* check for interruptable client segment.
	 */
	if (!scb_clseg(sidx) || !scb_intrseg(sidx))
	{
		return(0);
	}

	/* get addressability to u-block of the process.
	 */
	srsave = chgsr(TEMPSR,t->t_procp->p_adspace);
	mst = (struct mstsave *)( (TEMPSR << L2SSIZE) +
		((uint)&t->t_uthreadp->ut_save & SOFFSET) );

	/* call program check handler if not user mode.
	 */
	if (!v_isuser(mst))
	{
		/* Send the exception before making the thread
		 * runnable.
	 	 */
		vaddr = mst->except[EVADDR];

		/* save the vaddr for handling by vmexception().  this
		 * is necessary in case another page fault overwrites
		 * exmst->except[EORGVADDR].
		 */
		mst->o_vaddr = mst->except[EORGVADDR];

		p_slih(mst,vaddr,EINTR,t);
	}

	/* Call v_readyt to remove the thread from VMM waitlist.
	 */
	v_readyt(t->t_wchan2,t);

	/* restore segment register
	 */
	(void)chgsr(TEMPSR,srsave);

	return(0);
}

/*
 * getisidx(anch)
 *
 * determines a sidx based upon a page wait anchor
 * address.  in determining the sidx, only the anchors
 * associated with interruptable page waits will used.
 * this routine return an sidx or -1 (page wait type
 * is not interruptable).
 *
 * 
 */

static
getisidx(anch)
struct thread **anch;
{
	int	nfr,sid,sidx;

	/* check if page frame waitlist.  if so, determine nfr and
	 * get sid from pft. return sidx.
	 */
	if (&pft_waitlist(0) <= anch && anch <= &pft_waitlist(MAXRPAGE))
	{
		nfr = ((uint) anch - (uint) &pft_waitlist(0)) / 
			sizeof(struct pftsw);
		sid = pft_ssid(nfr);
		return(STOI(sid));
	}

	/* check if i/o level waitlist.  if so, return sidx.
	 */
	if (&scb_iowait(0) <= anch && anch <= &scb_iowait(pf_hisid -1))
	{
		sidx = ((uint) anch - (uint) &scb_iowait(0)) /
			 sizeof(struct scb);
		if (&scb_iowait(sidx) == anch)
		{
			return(sidx);
		}
	}

	/* check if message buffer waitlist.  if so, return sidx.
	 */
	if (&scb_waitlist(0) <= anch && anch <= &scb_waitlist(pf_hisid - 1))
	{
		sidx = ((uint) anch - (uint) &scb_waitlist(0)) / 
			sizeof(struct scb);
		if (&scb_waitlist(sidx) == anch)
		{
			return(sidx);
		}
	}

	/* page wait not of interruptable type.
	 */
	return(-1);

}
