static char sccsid[] = "@(#)44	1.14  src/bos/kernel/proc/profil.c, sysproc, bos411, 9428A410j 10/28/93 18:54:27";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: max
 *		min
 *		prof_off
 *		profbuf_chk
 *		profil
 *		
 *
 *   ORIGINS: 27,3,26
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


#include <sys/param.h>
#include <sys/limits.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <mon.h>
#include <sys/trchkid.h>
#include <sys/malloc.h>
#include <sys/errno.h>
#include <sys/syspest.h>
#include <sys/uio.h>
#include <sys/pseg.h>

void profbuf_chk(ulong begin, ulong size);
void prof_off();
void *sbrk();
#define min(a,b) ((ulong)(a)<(ulong)(b)?(ulong)(a):(ulong)(b))
#define max(a,b) ((ulong)(a)>(ulong)(b)?(ulong)(a):(ulong)(b))

/*
 * NAME: profil()
 *
 * FUNCTION: Execution Profiling
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a process
 *                        This routine may page fault
 *
 * RETURN VALUES:
 *		Void
 */

void
profil(
short *bufbase,
unsigned bufsize,
unsigned pcoffset,
unsigned pcscale)
{
	register struct prof *pprof;
	register int profdel;
	int wasprof;
	struct pinprof *pnprof;
	struct uthread *ut=curthread->t_uthreadp;
	int ipri;

	/* vars for non-traditional style profiling */
	int count = 1;
	caddr_t lhigh = 0;
	struct prof *kprof, *uprof;
	int isprof;

	/* allow only one thread in the process through at a time */
	simple_lock(&U.U_handy_lock);

	/*
	 * To remain compatible with other versions of
	 * UNIX, if 1 is specified turn profiling off.
	 * If 2 is specified, map all pc locations to
	 * one address producing a non-interrupting core
	 * clock.  In earlier releases of AIX, 0 was used
	 * to turn off profiling and 1 was used to map all
	 * pc location to one address.  In order to use the
	 * same code as AIX previous releases, the scale is
	 * decremented upon entry.
	 */
	if (pcscale == 1 || pcscale == 2) pcscale--;

	/* determine if profiling is currently turned on */
	if ((int)U.U_prof.pr_size != -1)
		wasprof = (U.U_prof.pr_scale != 0);
	else {
		wasprof = 0;
		for (pprof = (struct prof *)U.U_prof.pr_base;
			pprof->p_high; ++pprof)
			if (pprof->p_scale) {
				wasprof = 1;
				break;
			}
	}

	/*******************************************************************/
	/* if we're using traditional profiling arguments		   */
	/*******************************************************************/

	if ((short)bufsize != -1) {
		profdel = (pcscale != 0) - wasprof;

		/*
		 * if we are starting up profiling, 
		 * pin the buffer and attach to it
		 */
		if (profdel == 1)  { 
			if (bufbase == NULL)
				ut->ut_error = EFAULT;
			else {
				/* check addresses and pin buffer */
				profbuf_chk((ulong)bufbase,bufsize);
			}
			if (!ut->ut_error) {	

				/*
				 * keep track of pinned memory in u-block 
				 * so children can get to it
				 */
				pnprof = (struct pinprof *)xmalloc
					(sizeof (struct pinprof),4,pinned_heap);
				pnprof->count = 1;
				pnprof->pin_buf = bufbase;
				pnprof->pin_siz = bufsize;

				ipri = disable_lock(INTTIMER, &U.U_timer_lock);

				/* fill in u-block profiling fields */
				U.U_pprof = pnprof;
				U.U_prof.pr_scale = pcscale;
				U.U_prof.pr_base = bufbase;
				U.U_prof.pr_size = bufsize;
				U.U_prof.pr_off = pcoffset;

				unlock_enable(ipri, &U.U_timer_lock);
			}
		}

		/*
		 * if we are stopping profiling, 
		 * unpin buffer and detach
		 */ 
		else if (profdel == -1) { 

			/*
		 	* if user turned on profiling with -1 bufsize 
			* argument and is attempting to turn it off 
			* with non -1 bufsize, don't do anything
		 	*/
			if (U.U_prof.pr_size != (unsigned)-1) {
				prof_off();
			}
		}

		simple_unlock(&U.U_handy_lock);
				
		return ; /* we are through handling traditional arguments */
	}

	/*********************************************************************/
	/* we're using non-traditional profiling arguments                   */
	/* interpret profil arguments as:                                    */
	/*			profil(prof_structs, -1, 0, 0);		     */
	/*********************************************************************/


	/*
	 * determine how many prof structures there are in the user 
   	 * array, making sure the array is initialized properly 
	 */
	pprof = (struct prof *)xmalloc(sizeof (struct prof),4,kernel_heap);
	uprof = (struct prof *)bufbase;
	for (; ; ++uprof, lhigh = pprof->p_high, count++) {
		if (copyin((caddr_t)uprof,
			(caddr_t)pprof, sizeof (*pprof))) {
			ut->ut_error = EFAULT;
			break;
		}
		if (!pprof->p_high) 
			break;
		if (pprof->p_high <= lhigh) {
			ut->ut_error = EINVAL;
			break;
		}

	}
	xmfree((struct prof *)pprof, kernel_heap);

	/* if the user's array of prof structures was set up correctly */

	if (!ut->ut_error) {
		int hcount;

		/* copy the user array into kernel space */

		kprof = (struct prof *)xmalloc(sizeof(struct prof)*count, 
			4, pinned_heap);
		pprof = kprof;
		uprof = (struct prof *)bufbase;
		for (hcount = count; hcount > 0; ++uprof, ++pprof, hcount--) {
			if (copyin((caddr_t)uprof,
				(caddr_t)pprof, sizeof (*pprof))) {
				ut->ut_error = EFAULT;
				break;
			}

		/* if user wants us to calculate the scale for him, do it */

			if (pprof->p_scale == (unsigned)-1) {
				ulong text, bsize;
			
				text = (pprof->p_high - pprof->p_low
					+ sizeof (HISTCOUNTER)-1) / 
					    sizeof (HISTCOUNTER);
				bsize = pprof->p_bufsize / sizeof (HISTCOUNTER);
				if (text < bsize)
					bsize = text;
				if (bsize < 0x10000L)
					pprof->p_scale =
						((bsize << 16) - 1) / text;
				else
					pprof->p_scale = (bsize - 1) /
						((text + 0xffff) >> 16);
			}
		}

		/* see if the user is trying to turn on profiling */
		isprof = 0;
		if (count > 1)
			for (pprof = kprof; pprof->p_high; ++pprof)
				if (pprof->p_scale) {
					isprof = 1;
					break;
				}
		profdel = isprof - wasprof;

		/*
		 * if user is turning on profiling 
		 * and it wasn't already on
		 */
		if (profdel == 1) {
			ulong pbufsiz;
			ulong start ; 	/* starting addr for buffer */
			ulong end = 0;	/* ending addr of buffer */


			/* determine how big the profiling buffer is */
			for(pprof=kprof,hcount=count,
				start=(ulong)kprof->p_buff;
				hcount > 1; pprof++, hcount--) {

				/* get smallest buffer start addr */
				start = min(pprof->p_buff,start);
				/* get largest buffer end addr */
				end=max((ulong)(pprof->p_buff)+pprof->p_bufsize,
					end);
			}
			pbufsiz = end-start;
			
			/* if not turning off profiling */
			if (kprof[0].p_high)
				/* check addresses and pin buffer */
				profbuf_chk(start,pbufsiz);

			if (ut->ut_error || (count==1 && !kprof[0].p_high)){

				ipri = disable_lock(INTTIMER, &U.U_timer_lock);

				U.U_prof.pr_size = 0;
				U.U_prof.pr_scale = 0;
				U.U_prof.pr_base = NULL;
				U.U_prof.pr_off = 0;

				unlock_enable(ipri, &U.U_timer_lock);

				simple_unlock(&U.U_handy_lock);

				xmfree((char *)kprof, pinned_heap);

				return ;
			}
				
			/* 
			 * keep track of pinned memory in u-block 
			 * so children can get to it
			 */
			pnprof = (struct pinprof *)xmalloc
				(sizeof (struct pinprof),4,pinned_heap);
			pnprof->count = 1;
			pnprof->pin_buf = (short *)start;
			pnprof->pin_siz = pbufsiz;

			/*
			 * fill in profiling fields in u-block
			 */	
			ipri = disable_lock(INTTIMER, &U.U_timer_lock);

			U.U_prof.pr_scale = -1;		/* new style */
			U.U_pprof = pnprof;
			U.U_prof.pr_size = -1;
			U.U_prof.pr_off = 0;
			U.U_prof.pr_base = (short *)kprof;

			unlock_enable(ipri, &U.U_timer_lock);
		}

		/* user is turning off profiling and it was on */
		if (profdel == -1) {
			/* 
			 * toss our copy of the prof 
			 * buf the user passed in
			 */
			xmfree((char *)kprof, pinned_heap);

			/* 
			 * if correct kind of profiling
			 * then turn profiling off
			 */
			if (U.U_prof.pr_size == (unsigned)-1) {
				prof_off();
			}
		}
	}
	simple_unlock(&U.U_handy_lock);
}


/*
 * NAME:  prof_off()	
 *
 * FUNCTION: Turn execution profiling off for current process
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a process
 *                        This routine may page fault
 *
 * RETURN VALUES: Void
 *		Set u.u_error to EFAULT if unpinu() or xmdetach() fails
 *		xmfree()'s a number of profiling data structures
 */
void
prof_off()
{
	int new_style;		/* if !0 then args were passed */
				/* as an array of prof bufs */

	short *old_pin_buf;
	int old_pin_siz;
	long old_pin_count;
	short *old_pr_base;
	struct pinprof *old_pprof;
	int ipri;
	
	new_style = (U.U_prof.pr_size == (unsigned)-1) ? 1 : 0;

	ipri = disable_lock(INTTIMER, &U.U_timer_lock);

	/*
	 * turn off profiling 
	 */
	U.U_prof.pr_size = 0;
	U.U_prof.pr_scale = 0;
	U.U_pprof->count--;

	old_pr_base   = U.U_prof.pr_base;

	old_pin_buf   = U.U_pprof->pin_buf;
	old_pin_siz   = U.U_pprof->pin_siz;
	old_pin_count = U.U_pprof->count;
	old_pprof     = U.U_pprof;


	U.U_pprof = NULL;
	U.U_prof.pr_base = NULL;
	U.U_prof.pr_off = 0;

	unlock_enable(ipri, &U.U_timer_lock);

	/* 
	 * toss histogram buffer 
	 */
	ASSERT(old_pin_buf != NULL);
	if (unpinu((caddr_t)old_pin_buf, old_pin_siz, (short)UIO_USERSPACE))
			u.u_error = EFAULT;

	/* 
	 * if no no one else is profiling, 
	 * free allocated memory
	 */
	if (!old_pin_count) {
		/*
		 * if new_style toss area where prof bufs (arguments) were kept
		 */
		if(new_style)
			xmfree((struct prof *)old_pr_base, pinned_heap);
		/*
		 * toss pprof struct
		 */
		xmfree((struct pinprof *)old_pprof, pinned_heap);
	}
}

/*
 * NAME:  profbuf_chk()	
 *
 * FUNCTION: Check location of user's profiling buffer and pin it
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a process
 *                        This routine may page fault
 *
 * RETURN VALUES: Void
 */
void
profbuf_chk(ulong begin, ulong size)
{
	ulong end = begin+size;
	int segno = begin>>SEGSHIFT;
	struct uthread *ut=curthread->t_uthreadp;

	/* validate the segment number */
	if (segno < PRIVSEG || segno > BDATASEGMAX)
		ut->ut_error = EFAULT;

	/* must be a working storage segment */
	else if (!(segno == PRIVSEG || U.U_segst[segno].segflag & SEG_WORKING))
		ut->ut_error = EFAULT;

	/* can't be in the system part of the process private segment */
	else if(segno == PRIVSEG && (begin > USRSTACK || end > USRSTACK))
		ut->ut_error = EFAULT;

	/* pinu will enforce the segment limits */
	else if (pinu((caddr_t)begin, size, (short)UIO_USERSPACE)) 
		ut->ut_error = EFAULT;
}

