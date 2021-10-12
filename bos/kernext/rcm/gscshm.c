static char sccsid[] = "@(#)87  1.8  src/bos/kernext/rcm/gscshm.c, rcm, bos411, 9437A411a 9/8/94 17:51:48";

/*
 *   COMPONENT_NAME: (rcm) Rendering Context Manager User Buffer Mgmt.
 *
 *   FUNCTIONS: rcm_usr_buffer_init
 *		rcm_usr_buffer_extend
 *		rcm_usr_buffer_delete_set
 *		rcm_usr_buffer_detach_all
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989-1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/xmem.h>
#include <sys/rcm_win.h>
#include "rcmras.h"			/* error defines */
#include <sys/aixgsc.h> 		/* graphics system call declarations*/
#include <sys/rcm.h>			/* rendering context manager header */
#include <sys/sysmacros.h>		/* Macros for MAJOR/MINOR */
#include <sys/xmem.h>
#include <sys/uio.h>
#include <sys/malloc.h> 		/* memory allocation routines */
#include <sys/errno.h>			/* System error numbers */
#include <sys/user.h>			/* user structure */
#include <sys/adspace.h>
#include <sys/syspest.h>
#include "rcm_mac.h"			/* macros for rcm */
#include "xmalloc_trace.h"

BUGVDEF(dbg_gscshm, 0);

/*------------
  Manage access to user buffers for rcm and drivers.
  ------------*/

static int  attach_and_pin   (struct usrbuf *);
static void unpin_and_detach (struct usrbuf *);

/*
 *  rcm_usr_buffer_init - Perform first attachment to user buffer.
 *
 *  Can return std error codes (E* codes).
 *
 *  This is done only by the graphics process leader.
 *  No interthread protection is necessary.
 *
 *  However, any list updates or changes to flags that affect concurrent
 *  interrupt code must be properly ordered, unitary, or interrupt protected,
 *  as needed.
 */
int rcm_usr_buffer_init (pproc, set, start, length)
rcmProcPtr  pproc;
int  set;
caddr_t start;
int length;
{
    int  rc = 0;
    struct  usrbuf *ubuf;
    vmhandle_t  srval;

    if (set >= RCM_MAX_USR_BUFFER)
	return (EINVAL);

    /*  must be leader and have header array set up */
    if (!(pproc->procHead.flags & PROC_GP_LEADER)  ||
	pproc->procHead.pCproc->pusrbufhdr == NULL    )
	return (EINVAL);

    /*  pick up what's in the user segment register */
    srval = as_getsrval (getadsp (), start);

    /*  must be initial attachment  */
    if ((pproc->procHead.pCproc->pusrbufhdr)[set] != NULL)
	return (EINVAL);

    /*
     *  Malloc memory record, attach and pin, link into chain.
     */

    /*
     *  Create the memory record for this.
     */
    ubuf = xmalloc (sizeof (struct usrbuf), 3, pinned_heap);
    if (ubuf == NULL)
	return (ENOMEM);

    ubuf->start  = start;
    ubuf->length = length;
    ubuf->srval  = srval;

    /*
     *  Successfully performing this attachment also has the effect of
     *  rendering this piece of memory permanently accessible to us
     *  (for a later unpin) until we perform the cross memory detach.
     */
    rc = attach_and_pin (ubuf);

    if (rc != 0)
    {
	xmfree ((caddr_t) ubuf, pinned_heap);

	return (rc);
    }

    /* link record into the chain for drivers */
    /* this is unitary with respect to interrupts */
    ubuf->next                                = NULL;
    (pproc->procHead.pCproc->pusrbufhdr)[set] = ubuf;	/* *** unitary op *** */

    return (0);
}


/*
 *  rcm_usr_buffer_extend - Perform subsequent attachment to user buffer.
 *
 *  Can return std error codes (E* codes).
 *
 *  This is done only by the graphics process leader.
 *  No interthread protection is necessary.
 *
 *  However, any list updates or changes to flags that affect concurrent
 *  interrupt code must be properly ordered, unitary, or interrupt protected,
 *  as needed.
 */
int rcm_usr_buffer_extend (pproc, set, start, length)
rcmProcPtr  pproc;
int  set;
caddr_t start;
int  length;
{
    int  rc = 0;
    struct  usrbuf *ubuf;
    vmhandle_t srval;

    if (set >= RCM_MAX_USR_BUFFER)
	return (EINVAL);

    /*  must be leader and have header array set up */
    if (!(pproc->procHead.flags & PROC_GP_LEADER)  ||
	pproc->procHead.pCproc->pusrbufhdr == NULL    )
	return (EINVAL);

    /*  pick up what's in the user segment register */
    srval = as_getsrval (getadsp (), start);

    /*  must not be initial attachment  */
    ubuf = (pproc->procHead.pCproc->pusrbufhdr)[set];

    if (ubuf == NULL)
	return (EINVAL);

    /*
     *  Make sure we aren't overlaying something already registered.
     */
    while (ubuf != NULL)	/* exits with ubuf NULL, implies no err */
    {
	if (ubuf->srval == srval                     &&
	          start +       length > ubuf->start &&
	    ubuf->start + ubuf->length >       start    )
	{
	    break; /* exit loop, ubuf non-NULL */
	}

	ubuf = ubuf->next;
    }

    /* non-NULL means found overlap. */
    if (ubuf != NULL)
	return (EINVAL);

    /*  ubuf now scratch variable  */

    /*
     *  Malloc memory record, attach and pin, link into chain.
     */

    /*
     *  Create the memory record for this.
     */
    ubuf = xmalloc (sizeof (struct usrbuf), 3, pinned_heap);
    if (ubuf == NULL)
	return (ENOMEM);

    ubuf->start     = start;
    ubuf->length    = length;
    ubuf->srval     = srval;

    /*
     *  Successfully performing this attachment also has the effect of
     *  rendering this piece of memory permanently accessible to us
     *  (for a later unpin) until we perform the cross memory detach.
     */
    rc = attach_and_pin (ubuf);

    if (rc != 0)
    {
	xmfree ((caddr_t) ubuf, pinned_heap);

	return (rc);
    }

    /* link record into top of the chain */
    /* this is unitary with respect to interrupts */
    ubuf->next = (pproc->procHead.pCproc->pusrbufhdr)[set];
                 (pproc->procHead.pCproc->pusrbufhdr)[set] = ubuf;

    return (0);
}


/*
 *  rcm_usr_buffer_delete_set - Disconnect from a whole set.
 *
 *  Can return std error codes (E* codes).
 *
 *  This is done only by the graphics process leader.
 *  No interthread protection is necessary.
 *
 *  However, any list updates or changes to flags that affect concurrent
 *  interrupt code must be properly ordered, unitary, or interrupt protected,
 *  as needed.
 */
int rcm_usr_buffer_delete_set (pproc, set)
rcmProcPtr  pproc;
int  set;
{
    int  rc = 0;
    struct  usrbuf *ubuf, *ubuf_n;

    if (set >= RCM_MAX_USR_BUFFER)
	return (EINVAL);

    /*  must be leader and have header array set up */
    if (!(pproc->procHead.flags & PROC_GP_LEADER)  ||
	pproc->procHead.pCproc->pusrbufhdr == NULL    )
    {
	return (EINVAL);
    }

    /*
     *  There must be something to release.
     */
    ubuf = (pproc->procHead.pCproc->pusrbufhdr)[set];
    if (ubuf == NULL)
	return (EINVAL);

    /*
     *  Release all the blocks on the list.
     */
    (pproc->procHead.pCproc->pusrbufhdr)[set] = NULL;	/* *** unitary op *** */

    while (ubuf != NULL)	/* exits with ubuf NULL, implies no find */
    {
	ubuf_n = ubuf->next;

	/* pfrm unpin and xmdetach */
	unpin_and_detach (ubuf);

	/* throw away the struct */
	xmfree ((caddr_t) ubuf, pinned_heap);

	ubuf = ubuf_n;
    }

    return  0;
}


/*
 *  rcm_usr_buffer_detach_all - This function is used ONLY by the rcm
 *                              (at unmake-gp time) to close down all
 *                              the buffers owned by this process.
 */
void rcm_usr_buffer_detach_all (pcproc)
gscComProc *pcproc; 		/* pointer to common process structure */
{
    int  set;
    struct  usrbuf *ubuf, *ubuf_n;

    /*  must have header array set up */
    if (pcproc->pusrbufhdr == NULL)
	return;

    /*
     *  Release all our attachments.
     */
    for (set=0; set<RCM_MAX_USR_BUFFER; set++)
    {
	ubuf = (pcproc->pusrbufhdr)[set];

	(pcproc->pusrbufhdr)[set] = NULL;	/* *** unitary *** */

	while (ubuf != NULL)
	{
	    ubuf_n = ubuf->next;

	    /* pfrm unpin and xmdetach */
	    unpin_and_detach (ubuf);

	    /* throw away the struct */
	    xmfree ((caddr_t) ubuf, pinned_heap);

	    ubuf = ubuf_n;
	}
    }
}


/*
 *  attach_and_pin - Perform cross memory attachment and pin the block.
 *
 *  This function can return errors in standard format (E* codes).
 */
static int  attach_and_pin (ubuf)
struct usrbuf *ubuf;
{
    int  rc;

    /*
     *  Successfully performing this attachment also has the effect of
     *  rendering this piece of memory permanently accessible to us
     *  (for a later unpin) until we perform the cross memory detach.
     *
     *  The attachment should be performed before the pin (perhaps this
     *  isn't strictly required for the attachment phase, but we adhere
     *  to it because this is inscrutably bound up with the way the
     *  kernel works, and we learned to do it this way in dma_service.)
     */
    ubuf->xmemusr.aspace_id = XMEM_INVAL;
    rc = xmattach (ubuf->start, ubuf->length, &ubuf->xmemusr, USER_ADSPACE);

    if (rc == 0)			/* if xmattach worked */
    {
	/*
	 *  This uses pinu to get access to the user segments.  They are
	 *  presumed to be set up rightly, since we have just come in on the
	 *  system call.
	 */
	rc = pinu (ubuf->start, ubuf->length, UIO_USERSPACE);
	if (rc)
	    assert (!xmdetach (&ubuf->xmemusr));
    }

    return (rc);
}


/*
 *  unpin_and_detach - Unpin memory block and release cross mem attachment.
 *
 *  We follow the tradition established in dma_service of having no error
 *  returns.  We assert instead.  This has led in the past to 1) discovering
 *  the need to perform the unpin operation in the manner shown here, and 2)
 *  getting VMM bugs fixed.  If the previous tradition of ignoring the error
 *  returns had been used, those errors would not have been found and fixed.
 *  The system never crashes on those asserts any more, even though they
 *  are still in the system.
 */
static void unpin_and_detach (ubuf)
struct usrbuf *ubuf;
{
    caddr_t  newbase;

    /*
     *  We must unpin before detaching to avoid losing addressability
     *  to the memory, or having the vm object go away.
     *
     *  To detach successfully requires that we set up our own segment
     *  register.  This is because the original one may not still be
     *  set up the way it was when we xmattached and pinned.
     *
     *  See unpinu () in com/sys/ios/uio.c for the model for this.
     *  We assume the buffer doesn't span segments.  Note that we call
     *  unpin, not unpinu.  This is due to the fact that the segment
     *  register we allocate and use is in the system set, not the user
     *  set, as is assumed by unpinu.
     */
    newbase = vm_att (ubuf->srval, ubuf->start);

    assert (!unpin (newbase, ubuf->length));

    vm_det (newbase);

    /*
     *  Detach any valid cross mem attachment.
     */
    assert (!xmdetach (&ubuf->xmemusr));
}
