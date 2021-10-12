static char sccsid[] = "@(#)57        1.5  src/bos/kernel/ios/POWER/dispauth_paged.c, sysios, bos411, 9428A410j 6/29/94 15:23:53";
/*
 *   COMPONENT_NAME: SYSIOS
 *
 *   FUNCTIONS: reg_display_acc
 *		unreg_display_acc
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/dispauth.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/machine.h>
#include <sys/mstsave.h>
#include <sys/seg.h>
#include <sys/adspace.h>
#include <sys/syspest.h>
#include <sys/malloc.h>
#include <sys/lock_def.h>
#include <sys/buid.h>

#define  BID_TO_EXT(bid)	(bid & 0x0000000F)

extern void inval_all_dbats();

/***************************************************************************** 
 *
 * NAME: reg_display_acc
 *
 * FUNCTION:  Adds a busprt structure to the calling thread.
 *
 * EXECUTION ENVIRONMENT:
 *
 *       This service can be called by a program executing on
 *       the process level 
 *
 *       It may page fault.
 *
 * RETURN VALUE:
 *       0     if success
 *      -1     otherwise
 *
 * All busprt structures (for a thread) associated with domains controlled
 * by the same access control registers are chained under one gruprt structure.
 * All gruprt strustures (for a thread) are chained in a single-link list
 * anchored by thread.t_graphics.
 * All BAT type followed by SR type gruprt structures are kept in the front of
 * the list to speed up processing in SVC handler backend, resume(), and
 * kgetsig().
 *
 * Note that it is not necessary to disable dispatching before inserting a new 
 * gruprt structure as insertion to a single-link list can be performed
 * atomically. 
 * If the gruprt list is implemented as a double-link list, then it would be
 * necessary to disable dispatching (by disable external interrupts) before
 * insertion is performed. This is because dispatch() routine will run through
 * the list to invalidate display access rights when a thread is preempted.
 *
 * Also note that t_graphics is initialized to NULL in newthread() when it calls
 * pm_alloc() to allocate a new thread structure.
 *****************************************************************************
 */

int reg_display_acc(struct busprt *busprtp)
{
	register struct gruprt	*gp;        /* gruprt pointer                 */
	register struct gruprt  *lastbatsrgp;  /* the last bat/sr type gruprt */
	register char   type;               /* busprt type                    */
	register char   gtype;              /* gruprt type                    */
	register ushort segno;              /* display segment in the address *
					     * space of this process          */
	struct   thread *curt = curthread;  /* current thread pointer         */
	ulong    bid;                       /* bus id                         */
	ulong    ext;                       /* bus extention                  */
        ulong    rid;                       /* resource id                    */
	
	ASSERT(csa->prev == NULL);

	segno = BUSPRT_EADDR(busprtp) >> SEGSHIFT;
	type = BUSPRT_TYPE(busprtp);
	lastbatsrgp = NULL;

	/* find the corresponding gruprt structure
	 */
	switch (type)
	{
		case _BUSPRT_CSR15:

		bid = BID_TO_BUID(BUSPRT_IOCC_SR(busprtp));
		for (gp = curt->t_graphics; gp; gp = gp->gp_next)
		{
		     if (gp->gp_type == _BUSPRT_CSR15 &&
			 BID_TO_BUID(GRUPRT_IOCC_SR(gp)) == bid)  break;

		     /* find the last SR type gruprt structure */
		     gtype = gp->gp_type & _BUSPRT_MASK;
		     if (gtype == _BUSPRT_PBUS_TYPE)
		     {
			     lastbatsrgp = gp;
		     }
		}     
		break;
		
		case _BUSPRT_SR_MC:

		bid = BID_TO_BUID(BUSPRT_MC_SR(busprtp));
		ext = BID_TO_EXT(BUSPRT_MC_SR(busprtp));
		
		for (gp = curt->t_graphics; gp; gp = gp->gp_next)
		{
		     /* sanity check */
		     if (gp->gp_type == _BUSPRT_SR_MC && 
			 ((gp->gp_eaddr) >> SEGSHIFT) == segno)
		     {
			     assert(BID_TO_BUID(GRUPRT_MC_SR(gp)) == bid &&
				    BID_TO_EXT(GRUPRT_MC_SR(gp)) == ext);
			     break;
		     } 	     

		     /* find the last SR/BAT type gruprt structure */
		     gtype = gp->gp_type & _BUSPRT_MASK;
		     if (gtype == _BUSPRT_PBUS_TYPE || gtype == _BUSPRT_SR_MC ||
			 gtype == _BUSPRT_BAT_TYPE)
		     {
			     lastbatsrgp = gp;
		     }
		}     
		break;

		case _BUSPRT_PBUS_RSC:
		case _BUSPRT_PBUS_601:

		bid = BID_TO_BUID(BUSPRT_SR(busprtp));
		for (gp = curt->t_graphics; gp; gp = gp->gp_next)
		{
		     /* sanity check */
		     if (gp->gp_type == type &&
			 BID_TO_BUID(GRUPRT_SR(gp)) == bid)
		     {
			     assert(((gp->gp_eaddr) >> SEGSHIFT) != segno);
		     }

		     /* find the last SR/BAT type gruprt structure */
		     gtype = gp->gp_type & _BUSPRT_MASK;
		     if (gtype == _BUSPRT_PBUS_TYPE || gtype == _BUSPRT_SR_MC ||
			 gtype == _BUSPRT_BAT_TYPE)
		     {
			     lastbatsrgp = gp;
		     }
	        }
		break;

		case _BUSPRT_7F_XID:

		bid = BID_TO_BUID(BUSPRT_SR(busprtp));
                rid = BUSPRT_7F_XID_EAR(busprtp) & EAR_DISABLE_MASK;
		for (gp = curt->t_graphics; gp; gp = gp->gp_next)
		{
		     /* sanity check */	
		     if (gp->gp_type == type &&
			 BID_TO_BUID(GRUPRT_SR(gp)) == bid)
		     {
			     assert(((gp->gp_eaddr) >> SEGSHIFT) != segno);
		     }
		     
		     if (gp->gp_type == _BUSPRT_7F_XID)
		     {
		             assert((GRUPRT_7F_XID_EAR(gp) & EAR_DISABLE_MASK)
				    != rid);
		     }	 
		     else {
	                 if (gp->gp_type == _BUSPRT_BAT_XID)
			 {
			     assert((GRUPRT_BAT_XID_EAR(gp) & EAR_DISABLE_MASK)
				    != rid);
		         }
		     }
		     
		     gtype = gp->gp_type & _BUSPRT_MASK;

		     /* find the last SR/BAT type gruprt structure
		      */ 	
		     if (gtype == _BUSPRT_PBUS_TYPE || gtype == _BUSPRT_SR_MC ||
			 gtype == _BUSPRT_BAT_TYPE)
		     {
			     lastbatsrgp = gp;
		     }
	        }
		break;
		
		case _BUSPRT_BAT:

		/* at most 4 BAT type displays registered */
		if (GRUPRT_NBAT((struct gruprt *)curt->t_graphics) == 4)
			return(-1);
		
		for (gp = curt->t_graphics; gp; gp = gp->gp_next)
		{
			/* find the last BAT type gruprt structure */
			if ((gp->gp_type & _BUSPRT_MASK) == _BUSPRT_BAT_TYPE)
		     {
			     lastbatsrgp = gp;
		     }
	        }
		break;

		case _BUSPRT_BAT_XID:

		/* at most 4 BAT type displays registered */
		if (GRUPRT_NBAT((struct gruprt *)curt->t_graphics) == 4)
			return(-1);

                rid = BUSPRT_BAT_XID_EAR(busprtp) & EAR_DISABLE_MASK;
		
		for (gp = curt->t_graphics; gp; gp = gp->gp_next)
		{
		     /* sanity check */	
		     if (gp->gp_type == _BUSPRT_7F_XID)
		     {
		             assert((GRUPRT_7F_XID_EAR(gp) & EAR_DISABLE_MASK)
				    != rid);
		     }	 
		     else {
	                 if (gp->gp_type == _BUSPRT_BAT_XID)
			 {
			     assert((GRUPRT_BAT_XID_EAR(gp) & EAR_DISABLE_MASK)
				    != rid);
		         }
		     }

		     /* find the last BAT type gruprt structure
		      */ 	
		     if ((gp->gp_type & _BUSPRT_MASK) == _BUSPRT_BAT_TYPE)
		     {
			     lastbatsrgp = gp;
		     }
	        }
		break;
		
		default: assert(0);
	}

	/* corresponding gruprt structure does not exist
	 */  
	if (gp == NULL) 
	{
	    gp = xmalloc(sizeof(struct gruprt), 4, pinned_heap);
	    if (gp == NULL)
	    {
		return(-1);
	    }

	    /* initialize gruprt structure
	     */
	    gp->gp_owner = curt; 
	    gp->gp_type = busprtp->bp_type;
	    gp->gp_own  = 0;
	    gp->gp_eaddr = segno << SEGSHIFT;
	    gp->gp_primxid = 0;
	    gp->gp_nbat = 0;
	    gp->gp_busprt = NULL;
	    
	    /* initialize shadow copy of hardware access control registers
	     */  
	    switch(type)
	    {
		    case _BUSPRT_CSR15:
		    /* make sure T = 1, K = 0, addr chk = 1, addr inc = 1,
	             * reserved bits (14-23) = 0, IOCC select = 1,
		     * RT compat bit = 0, bypass = 1, and EXT bits (28-31) = 0
	             */  
	    	    BUSPRT_IOCC_SR(busprtp) =  
			     (BUSPRT_IOCC_SR(busprtp) & 0x3FF00000) | 0x800C00A0;

		    GRUPRT_IOCC_SR(gp) = BUSPRT_IOCC_SR(busprtp);
		    GRUPRT_CSR15(gp)   = INVCSR15;
		    GRUPRT_LIMIT(gp)   = INVBUSLIMT;
		    break;

		    case _BUSPRT_SR_MC:
		    /* make sure Ks = Kp = 0
		     */  
		    BUSPRT_SR(busprtp) &= 0x9FFFFFFF;
		    GRUPRT_SR(gp)      = INV_MC_SEGREG;
		    break;
		    
		    case _BUSPRT_PBUS_RSC:
		    GRUPRT_SR(gp)      = INV_PB_SEGREG_PWR;
		    break;
		    
		    case _BUSPRT_PBUS_601:
		    GRUPRT_SR(gp)      = INV_PB_SEGREG_PPC;  
		    break;

		    case _BUSPRT_7F_XID:
		    GRUPRT_7F_XID_SR(gp) = INV_PB_SEGREG_PPC;
		    GRUPRT_7F_XID_EAR(gp) = BUSPRT_7F_XID_EAR(busprtp) &
			                    EAR_DISABLE_MASK;
		    break;
			    
		    case _BUSPRT_BAT:
		    GRUPRT_BAT_BATU(gp) = BUSPRT_BAT_BATU(busprtp) &
			                  BAT_DISABLE_MASK;
		    GRUPRT_BAT_BATL(gp) = BUSPRT_BAT_BATL(busprtp);
		    break;
		    
		    case _BUSPRT_BAT_XID:
		    GRUPRT_BAT_XID_BATU(gp) = BUSPRT_BAT_XID_BATU(busprtp) &
			                      BAT_DISABLE_MASK;
	            GRUPRT_BAT_XID_BATL(gp) = BUSPRT_BAT_XID_BATL(busprtp);
		    GRUPRT_BAT_XID_EAR(gp)  = BUSPRT_BAT_XID_EAR(busprtp) &
			                      EAR_DISABLE_MASK;
		    break;
		    
		    default:
		    assert(0);
	    }

	    /* Insert new gruprt structure after the last existing BAT/SR type
	     * gruprt structure.
	     * The order between inserting new gruprt structure and
	     * initializing access control registers is important as we are
	     * running at INTBASE level
	     */  

	    if (lastbatsrgp == NULL)
	    {
		((volatile gruprt_t *)gp)->gp_next = curt->t_graphics;   
		curt->t_graphics = gp;
	    }
	    else {
		((volatile gruprt_t *)gp)->gp_next = lastbatsrgp->gp_next;
		lastbatsrgp->gp_next = gp;
	    }     

	    /* Initialize csr15 and limit register when the first busprt
	     * structure associated with an IOCC is added for a thread.
	     * This is required because this graphics thread may start
	     * accessing the display in the user mode after returning from
	     * svc before csr15 and limit register are initialized in
	     * dispatch().
	     */
	    if (type == _BUSPRT_CSR15)
	    {
		INIT_CSR15_LMT(gp);
	    }
	    else {
		/* increase BAT count in the first gruprt structure
		 * Note that state_save()/resume() should not rely on
		 * the bat count.  This is because the bat count in the first
		 * gruprt structure may be one less what is really in the list
		 * if the process is interrupted at this point
		 */   
		if ((type & _BUSPRT_MASK) == _BUSPRT_BAT_TYPE)
		{
		     	(GRUPRT_NBAT((struct gruprt *)curt->t_graphics))++;
		}
	    }
        }
	else {
	    /* only CSR15 and SR_MC type may have multiple busprt structures
	       under one gruprt
	     */  
	    ASSERT(type == _BUSPRT_CSR15 || type == _BUSPRT_SR_MC);
        }
	
	/* initialize busprt structure and insert it under its gruprt 
	 */
	busprtp->bp_grp = gp;
	busprtp->bp_next = gp->gp_busprt;
	gp->gp_busprt = busprtp;
	busprtp->bp_set = _BUSPRT_NO_ACCESS;
	return(0);
}


/***************************************************************************** 
 *
 * NAME: unreg_display_acc
 *
 * FUNCTION:  remove a busprt structure from the calling thread
 *
 * EXECUTION ENVIRONMENT:
 *
 *       This service can be called by a program executing on
 *       the process level 
 *
 *       It may page fault.
 *
 *****************************************************************************
 */

void unreg_display_acc(struct busprt *busprtp)
{  
	register struct gruprt  *gp;        /* target gruprt pointer          */
	register struct gruprt  *prevgp;    /* previous gruprt pointer        */
	register struct	busprt	*prevbp;    /* previous busprt pointer        */
	struct busprt	        *first;     /* first busprt pointer           */
	struct   thread *curt = curthread;  /* current thread pointer         */
	
	ASSERT(csa->prev == NULL);

	gp = busprtp->bp_grp;
	
        /* make sure access rights is revoked */
	if (busprtp->bp_set != _BUSPRT_NO_ACCESS)
	    revoke_display_owner(busprtp);

	first = gp->gp_busprt;

	if (first == busprtp)
	{
	    /* remove the busprt structure from the list */	
	    gp->gp_busprt = busprtp->bp_next;

	    /* check if the last busprt in the list */
	    if (gp->gp_busprt == NULL)
	    {
		/* decrement BAT count and invalidate ALL DBATS
		 * Note that state_save()/resume() should not rely on
		 * the bat count.  This is because the bat count in the first
		 * gruprt structure may be one less what is really in the list
		 * if the process is interrupted at this point
		 */ 
		if ((gp->gp_type & _BUSPRT_MASK) == _BUSPRT_BAT_TYPE)
		{
		     	(GRUPRT_NBAT((struct gruprt *)curt->t_graphics))--;

			/* It is necessary to invalidate the last dbat
			 * currently in use.  This is because when dbats are
			 * set up on returning to the user mode we start with
			 * dbat0.  We choose to invalidate all dbats here for
			 * lack of indirct addressing in mtspr instruction.
			 */
			inval_all_dbats();
		}

		/* remove the gruprt structure from the list */
		if (gp == curt->t_graphics)
		{
		    /* save the bat count in the 2nd gruprt structure
		     * if there exists one  
		     */	
		    if (gp->gp_next)
		    {
			GRUPRT_NBAT(gp->gp_next) = GRUPRT_NBAT(gp);
		    }	
		    curt->t_graphics = gp->gp_next;
	        }
		else {
		    for (prevgp = curt->t_graphics; prevgp;
			 prevgp = prevgp->gp_next)
		    {
			 if (prevgp->gp_next == gp)
			 {
				 prevgp->gp_next = gp->gp_next;
				 break;
			 }	 
		    }
		    ASSERT(prevgp);
		}
		/* free the gruprt structure */
		xmfree(gp, pinned_heap);
	    }	    
        }
	else {
	    /* remove the busprt structure from the list */
	    for (prevbp = first; prevbp; prevbp = prevbp->bp_next)
	    {	    
		 if (busprtp == prevbp->bp_next)
		 {   
		     prevbp->bp_next = busprtp->bp_next;
		     break;
	         }
            } 
	    ASSERT(prevbp);
        }
}
	








