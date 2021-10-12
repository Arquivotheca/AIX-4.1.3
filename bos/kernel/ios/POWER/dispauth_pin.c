static char sccsid[] = "@(#)58        1.2  src/bos/kernel/ios/POWER/dispauth_pin.c, sysios, bos411, 9428A410j 5/5/94 19:10:27";
/*
 *   COMPONENT_NAME: SYSIOS
 *
 *   FUNCTIONS: grant_display_owner (exported)
 *		revoke_display_owner (exported)
 *              setup_display_acc
 *		dispatch_graphics_inval
 *		dispatch_graphics_setup
 *
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

#include <sys/dispauth.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/machine.h>
#include <sys/mstsave.h>
#include <sys/seg.h>
#include <sys/adspace.h>
#include <sys/syspest.h>
#include <macros.h>
#include <sys/vmuser.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/seg.h>
#include <sys/inline.h>

extern void mtear();
extern int disable_ints();
extern void enable_ints();

/***************************************************************************** 
 *
 * NAME: grant_display_owner
 *
 * FUNCTION:  grant display ownership to a thread and conditionally
 *            set up display access rights for the new owner
 * 
 * EXECUTION ENVIRONMENT:
 *
 *       This routine can be called from both the process environment and
 *       the interrupt environment.
 *
 *       It may not page fault.
 *
 * RETURN VALUE:  no return value
 *
 *****************************************************************************
 */


void grant_display_owner(struct busprt *busprtp)
{
	int	oldpri;		/* old interrupt priority                    */
	struct gruprt *gp;      /* gruprt structure pointer                  */ 

	gp = busprtp->bp_grp;
	
        /* disable external interrupts */
        oldpri = disable_ints();

	if (busprtp->bp_set == _BUSPRT_NO_ACCESS)
        {
            busprtp->bp_set = busprtp->bp_type;
            (gp->gp_own)++;
            SET_ACWS(gp,busprtp);
        }

        if (gp->gp_owner == curthread)
        {   /* called under the gruprt structure owner.
               establish display access rights.
             */
            setup_display_acc(gp);
        }

	/* re-enable external interrupts */
	enable_ints(oldpri);
}


/***************************************************************************** 
 *
 * NAME: revoke_display_owner
 *
 * FUNCTION:  revoke display ownership from a thread and conditionally
 *            invalidate display access rights from the thread
 * 
 * EXECUTION ENVIRONMENT:
 *
 *       This routine can be called from both the process environment and
 *       the interrupt environment.
 *
 *       It may not page fault.
 *
 * RETURN VALUE:  0   --  revoke done
 *               -1   --  revoke failed (in SMP)
 *
 *****************************************************************************
 */

int revoke_display_owner(struct busprt *busprtp)
{
	int	oldpri;	        /* old interrupt priority                    */
	struct gruprt *gp;      /* gruprt structure pointer                  */ 
	struct  thread   *curt = curthread;

	gp = busprtp->bp_grp;
	
        /* disable external interrupts */
        oldpri = disable_ints();

	if (busprtp->bp_set != _BUSPRT_NO_ACCESS)
        {
            busprtp->bp_set = _BUSPRT_NO_ACCESS;
            (gp->gp_own)--;
            RESET_ACWS(gp,busprtp);
        }

        if (gp->gp_owner == curt)
	{
            /* called under the gruprt structure owner.
	       Invalidate display access rights
	     */	
            setup_display_acc(gp);
        }

        /* re-enable external interrupts */
        enable_ints(oldpri);

        return (0);
}


/***************************************************************************** 
 *
 * NAME: setup_display_acc
 *
 * FUNCTION:  set up display access rights according to the shadow
 *	      access control register values in the input gruprt structure
 * 
 * INPUT:
 *            gp, pointer to a gruprt structure (locked in SMP)
 *
 * RETURN VALUE: None
 *
 * EXECUTION ENVIRONMENT:
 *
 *       This routine can be called from both the process environment and
 *       the interrupt environment.
 *       External interrupts are fully disabled.
 *
 *       It may not page fault.
 *
 *	 Called by grant_display_owner(), revoke_display_owner(), 
 *       dispatch_graphics_setup(), io_exception(), MASTER_FLUSH(),
 *       d_bflush(), d_move() 
 *****************************************************************************
 */

void setup_display_acc(
	struct gruprt *gp)
{
	register ulong   ioccaddr;       /* IO handle            */
	volatile ulong   *csr15p;        /* CSR 15               */
	volatile ulong   *limitsp;       /* IOCC limits register */
	register char    mtype; 	 /* busprt type          */
	
	mtype = gp->gp_type & _BUSPRT_MASK;
     
	switch (mtype) {

	    case _BUSPRT_CSR15:

	    /* gain access to IOCC address space with GRUPRT_IOCC_SR(gp)
	     */
	    ioccaddr = (ulong)io_att(GRUPRT_IOCC_SR(gp), 0);
	    csr15p = (ulong *)(ioccaddr + CSR15);
	    limitsp = (ulong *)(ioccaddr + LIMIT_REG);

	    /* store new csr15 and limits register */
	    *csr15p = GRUPRT_CSR15(gp);
	    *limitsp = GRUPRT_LIMIT(gp);

	    /* release access to IOCC address space */
	    io_det(ioccaddr);
	    break;

	    case _BUSPRT_7F_XID:

	    /* set up the ear register if this is the primary xid display
	     */
	    if (GRUPRT_PRIMXID(gp))
	    {
		mtear(GRUPRT_7F_XID_EAR(gp));
	    }
	    break;

	    case _BUSPRT_BAT_XID:

	    /* set up the ear register if this is the primary xid display
	     */
	    if (GRUPRT_PRIMXID(gp))
	    {
		mtear(GRUPRT_BAT_XID_EAR(gp));
	    }
	    break;
    }
}


/***************************************************************************** 
 *
 * NAME: dispatch_graphics_inval()
 *
 * FUNCTION: invalidate display access rights for a preempted graphics thread 
 *
 * INPUT: pointer to the first gruprt structure for the prempted graphics
 *	  thread
 *
 * RETURNS: NONE
 *
 * EXECUTION ENVIRONMENT:
 *
 *       This routine is called only from dispatch() in the interrupt
 *	 environment.
 *       External interrupts are fully disabled.
 *
 *       It may not page fault.
 *
 *****************************************************************************
 */

void dispatch_graphics_inval(
	register struct gruprt *gp)	        /* gruprt structure pointer */
{
	register ulong   ioccaddr;       /* IO handle            */
	volatile ulong   *csr15p;        /* CSR 15               */
	volatile ulong   *limitsp;       /* IOCC limits register */

	for ( ; gp; gp = gp->gp_next)
	{
	    switch(gp->gp_type)
	    {
		    case _BUSPRT_CSR15:
		    /* gain access to IOCC address space with GRUPRT_IOCC_SR(gp)
		     */
		    ioccaddr = (ulong)io_att(GRUPRT_IOCC_SR(gp), 0);
		    csr15p = (ulong *)(ioccaddr + CSR15);
		    limitsp = (ulong *)(ioccaddr + LIMIT_REG);

		    /* invalidate csr15 and limits register */
		    *csr15p = INVCSR15;
		    *limitsp = INVBUSLIMT;
		    
		    /* release access to IOCC address space */
		    io_det(ioccaddr);
		    break;

		    /* invalidate EAR */
		    case _BUSPRT_7F_XID:
		    case _BUSPRT_BAT_XID:
		    if (GRUPRT_PRIMXID(gp))	mtear(0);
		    break;
	    }	    
	}
}



/***************************************************************************** 
 *
 * NAME: dispatch_graphics_setup()
 *
 * FUNCTION: setup display access rights for a preempting graphics thread
 *
 * INPUT: pointer to the first gruprt structure for the prempted graphics
 *	  thread
 *
 * RETURNS: NONE
 *
 * EXECUTION ENVIRONMENT:
 *
 *       This routine is called only from dispatch() in the interrupt
 *	 environment.
 *       External interrupts are fully disabled.
 *
 *       It may not page fault.
 *
 *****************************************************************************
 */

void dispatch_graphics_setup(
	register struct gruprt *gp)
{	
	/* setup display access rights according to the shadow access control
	 * register values in each gruprt structures
	 */  
	for ( ; gp; gp = gp->gp_next)
	{
	     setup_display_acc(gp);
        }
}
