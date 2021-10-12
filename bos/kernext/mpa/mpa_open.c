static char sccsid[] = "@(#)87	1.3  src/bos/kernext/mpa/mpa_open.c, sysxmpa, bos411, 9428A410j 2/2/94 12:53:34";
/*
 *   COMPONENT_NAME: (SYSXMPA) MP/A SDLC DEVICE DRIVER
 *
 *   FUNCTIONS: free_open_struct
 *		mpa_open
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <errno.h>
#include <sys/ioacc.h>
#include <sys/adspace.h>
#include <sys/device.h>
#include <sys/ioctl.h>
#include <sys/malloc.h>
#include <sys/mpadd.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>
#include <sys/errids.h>


mpa_open ( dev_t                 dev,
	  unsigned long         devflag,
	  int                   chan,
	  struct kopen_ext      *p_ext )

{
	struct acb      *acb;           /* ptr to adapter control block */
	struct intr     *p_intr;        /* work ptr                     */
	int             rc=0;           /* general return code          */

        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
        ³ put the trace hook for open entry ³
        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	DDHKWD5(HKWD_DD_MPADD, DD_ENTRY_OPEN, 0, dev, 0, 0, 0, 0);

	if ( !(devflag & DKERNEL) ) 
	      return EACCES; /* Deny access to USER processes */
	if((acb = get_acb(minor(dev))) == NULL)
	      return EINVAL;
	if (lockl(&acb->adap_lock, LOCK_SIGRET) != LOCK_SUCC)
	      return EINTR;
	if (acb->num_opens>1)    /* allow only one open */
	      return EBUSY;
	    
	acb->dev = dev; /* set major/minor device number */
	OPENP.op_chan = chan;
	OPENP.op_mode = devflag;     /* device flags opened with */

        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
        ³ If this is the first open for this adapter    ³
        ³ allocate and initialize the necessary data    ³
        ³ structures and make sure the driver is pinned.³
        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        if (acb->num_opens == 0) 
	{
                   if ( rc=mpa_init_dev(acb) ) 
		   {
                          OPENP.op_flags &= ~OP_OPENED;
                          unlockl(&acb->adap_lock);
		  	  DDHKWD5( HKWD_DD_MPADD, DD_EXIT_OPEN, EIO, dev, 0,0,0,0);
                          return EIO;
                   }
        }
        acb->num_opens++;

	OPENP.mpa_kopen.rx_fn = p_ext->rx_fn;  /* receive function */
	OPENP.mpa_kopen.stat_fn = p_ext->stat_fn;  /* status function */
	OPENP.mpa_kopen.tx_fn = p_ext->tx_fn;  /* transmit function */
	OPENP.mpa_kopen.open_id = p_ext->open_id;  /* open identifier */
	p_ext->status = CIO_OK; /*set status to OK */


	unlockl(&acb->adap_lock);

        MPATRACE3("OpnX",dev,rc);
        DDHKWD5( HKWD_DD_MPADD, DD_EXIT_OPEN, rc, dev, 0,0,0,0);
	return(rc);
}  /* mpa_open() */

/*****************************************************************************
** NAME:        free_open_struct
**
** FUNCTION:
**
** EXECUTION ENVIRONMENT:  Adapter lock is held in the process environment.
**
** CALLED BY:	mpa_close(), and alloc_resources
**
** NOTES:
**
** RETURNS:     0 - Success
**
*****************************************************************************/
void
free_open_struct(
	struct acb *acb)
{
	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ If this was the last open for this adapter ³
	³ de-allocate the data structures.           ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if (--acb->num_opens == 0) 
	{
		mpa_term_dev(acb);
	}

	return;
} /* free_open_struct() */
