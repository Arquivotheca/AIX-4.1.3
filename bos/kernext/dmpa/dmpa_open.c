static char sccsid[] = "@(#)75 1.1 src/bos/kernext/dmpa/dmpa_open.c, sysxdmpa, bos411, 9428A410j 4/30/93 12:49:13";
/*
 *   COMPONENT_NAME: (SYSXDMPA) MP/A DIAGNOSTICS DEVICE DRIVER
 *
 *   FUNCTIONS: free_open_struct
 *		mpaopen
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
#include <sys/dmpauser.h>
#include <sys/dmpadd.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>
#include <sys/errids.h>


mpaopen ( dev_t                 dev,
	  unsigned long         devflag,
	  int                   chan,
	  struct kopen_ext      *p_ext )

{
	struct acb      *acb;         /* ptr to adapter control block */
	struct intr     *p_intr;        /* work ptr                     */
	int             rc=0;           /* general return code          */

	/* put the trace hook for open entry */


	DDHKWD5(HKWD_DD_MPADD, DD_ENTRY_OPEN, 0, dev, 0, 0, 0, 0);

	if((acb = get_acb(minor(dev))) == NULL)
	      rc = ENXIO;
	else if (lockl(&acb->adap_lock, LOCK_SIGRET) != LOCK_SUCC)
	      rc = EINTR;
	else if (acb->num_opens>1)    /* allow only one open */
	      rc = EBUSY;
	else {
	      /* set major/minor device number */
	      acb->dev = dev;
	      OPENP.op_chan = chan;
	      OPENP.op_mode = devflag;     /* device flags opened with */

	      if ( OPENP.op_mode & DKERNEL ) {

		      /* receive function */
		      OPENP.mpa_kopen.rx_fn = p_ext->rx_fn;

		      /* status function */
		      OPENP.mpa_kopen.stat_fn = p_ext->stat_fn;

		      /* transmit function */
		      OPENP.mpa_kopen.tx_fn = p_ext->tx_fn;

		      /* open identifier */
		      OPENP.mpa_kopen.open_id = p_ext->open_id;

		      p_ext->status = CIO_OK; /*set status to OK */

	      }

	      unlockl(&acb->adap_lock);
	}

	DDHKWD1(HKWD_DD_MPADD, DD_EXIT_OPEN,rc, dev);
	return(rc);
}  /* mpaopen() */
/*****************************************************************************
** NAME:        free_open_struct
**
** FUNCTION:
**
** EXECUTION ENVIRONMENT:
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
	int      spl;

	DISABLE_INTERRUPTS(spl);

	/*
	** If this was the last open for this adapter
	** de-allocate the data structures.
	*/
	if (--acb->num_opens == 0) {
		mpa_term_dev(acb);
	}

	bzero(OPENP, sizeof(open_t));

	ENABLE_INTERRUPTS(spl);
	return;
} /* free_open_struct() */





