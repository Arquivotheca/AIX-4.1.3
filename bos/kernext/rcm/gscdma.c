static char sccsid[] = "@(#)75	1.45  src/bos/kernext/rcm/gscdma.c, rcm, bos411, 9437A411a 9/8/94 17:50:23";

/*
 * COMPONENT_NAME: (rcm) Rendering Context Manager Dma Service
 *
 * FUNCTIONS:
 *
 * This main line file contains the graphics device driver interface
 * routines:
 *
 * dma_service - perform a dma read/write or poll
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989-1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include <lft.h>                    /* includes for all lft related data */
#include <sys/adspace.h>
#include <sys/syspest.h>
#include <sys/sleep.h>            /* for e_sleep, e_wakeup            */
#include <sys/sysmacros.h>        /* Macros for MAJOR/MINOR           */
#include <sys/dma.h>              /* direct memory addressing         */
#include <sys/uio.h>              /* user i/o                         */
#include <sys/malloc.h>           /* memory allocation routines       */
#include <sys/user.h>             /* user structure                   */
#include "gscsubr.h"              /* rcm stuff                        */
#include "rcm_mac.h"              /* macros for rcm                   */
#include "rcmhsc.h"               /* RCM heavy switch ctlr structures */
#include "xmalloc_trace.h"
 
BUGVDEF(dbg_gscdma, 99)
 
#define  CALLC(S)	if (rc = (S))  assert (0);
#define  CALLR(S)	if (rc = (S))  return (rc);
#define  CALLRC(S,RC)	if (rc = (S))  return (RC);

/*
 *  Function entry definitions.
 */
int  dma_free (gscDev *, struct _rcmProc *);

static void dma_end_master (struct _rcmProc *);
static void dma_end_slave  (struct _rcmProc *);

int  dma_service  (struct phys_displays *, struct _gscdma *);
int  dma_service1 (struct phys_displays *,
			gscDev *, struct _rcmProc *, struct _gscdma *);

static int  take_down_sw    (struct sw *, struct xmem *, struct _gscdma_ext *);
static int  set_up_sw       (struct sw *, struct sw *, struct xmem *,
						struct _gscdma_ext *);
static int  alter_subwindow (struct sw *, struct sw *, struct xmem *,
						struct _gscdma_ext *, int);

static void wait_lock         (gscDev *, struct _rcmProc *);
static void wait_lock_if_ours (gscDev *, struct _rcmProc *);
static void wait_end_dma      (gscDev *, struct _rcmProc *);
static void release_lock      (gscDev *, struct _rcmProc *);

/*
 *  Whereas the first word of the gscdma structure is an error code to
 *  be returned to the user, we actually use it for another purpose when
 *  the structure is exclusively ours:  It is a pointer to the following
 *  structure used to house miscellaneous items we need.  This kloodge is
 *  to avoid changing the layout of the preexisting structures at this time.
 *
 *  Currently this structure contains the following miscellaneous items:
 *
 *  1)  The DMA command string on a per process basis.
 *  2)  Information for the end-dma interrupt handlers.
 */
struct _miscel
{
	long  cmd_length;	/* length of dma_cmd string */
	char *dma_cmd;		/* pointer to dma command or NULL */
	int   error;		/* error code for callback use */
	int   cur_win;		/* slave current window in processing */
	int   seq;		/* sequence number for callback control */
};

/*
 *  dma_free - Free up allocated memory, pinned resources, and DMA
 *	       facilities.
 *
 *  This will only be called from process level to release resources.
 *  This will return any error from a d_complete call for both masters
 *  and slaves.
 */
int dma_free (pdev, pproc)
gscDev *pdev;               /* pointer to gsc device structure    */
struct _rcmProc *pproc;
{
    int i;
    int  rc = 0;
    struct _gscdma  *pRCM;	/* point to permanent DMA window info */
    struct sw  *pRW;		/* point to subwindow structure */
    struct xmem *pXM;
    struct _gscdma_ext  *pSG;
    int  master, err;
    struct _miscel  *pmiscel;
 
    RCM_TRACE (0x799, getpid (), pdev, pproc);

    pRCM = &pproc->procHead.gscdma;
    pmiscel = (struct _miscel *) pRCM->error;

    master = !(pdev->devHead.display->dma_characteristics & DMA_SLAVE_DEV);

    /*
     *  Wait for any process DMA in progress to finish.  Ignore
     *  signals while waiting.
     */
    wait_end_dma (pdev, pproc);

    /*
     *  Detach and unpin any pinned areas for this process.
     *  No error is possible.
     */
    i   = 0;
    pRW = &pRCM->subwindow[0];
    pXM = &pproc->procHead.xmemdma[0];
    pSG = &pproc->procHead.gscdma_ext[0];
    for ( ; i<MAX_SUBAREAS; i++,pRW++,pXM++,pSG++)
    {
	if (pRW->sw_addr != NULL)
	    take_down_sw (pRW, pXM, pSG);
    }

    /*
     *  Free DMA command string and miscellaneous structure.
     */
    rc = 0;
    if (pmiscel != NULL)
    {
        /*
         *  Check d_complete error code from interrupt level.
         */
        rc = pmiscel->error;

        if (pmiscel->dma_cmd != NULL)
	    xmfree (pmiscel->dma_cmd, pinned_heap);

        xmfree ((caddr_t) pmiscel, pinned_heap);
    }

    pRCM->error = NULL;

    return  rc;
}


/*
 *  dma_end_master - Master DMA termination routine.
 *
 *  There is no per-subwindow processing.  The only thing this does
 *  to the DMA subsystem is the d_complete call to flush buffers and
 *  check for errors.
 */
static void  dma_end_master (pproc)
struct _rcmProc *pproc;
{
    int  *error;
    struct _miscel  *pmiscel;
    gscDev *pdev;               /* pointer to gsc device structure    */
    struct _gscdma  *pRCM;	/* point to permanent DMA window info */
    struct sw  *pRW;		/* point to subwindow structure */
    struct xmem  *pXM;		/* point to cross mem attach structure */
    struct phys_displays  *pDS;
    int     i;			/* current window number */
    ulong   use_start_flag;
 
    pRCM     = &pproc->procHead.gscdma;		/* kernel dma record */
    pmiscel  = (struct _miscel *) pRCM->error;	/* miscel struct ptr */
    error    = &pmiscel->error;		/* place to store error */
    pdev     = pproc->procHead.pGSC;

    RCM_TRACE (0x749, getpid (), pdev, pproc);

    pmiscel->seq++;			/* update callback sequence */

    use_start_flag = pRCM->flags & DMA_START_FLAG;

    /*
     *  Call d_complete for each subwindow.  Save the first error code
     *  from the d_complete call.
     */
    pRW = pRCM->subwindow;		/* sw info for 0'th window */
    pXM = pproc->procHead.xmemdma;
    pDS = pdev->devHead.display;

    *error = 0;

    for (i=0; i<pRCM->num_sw; i++,pRW++,pXM++) /* go down all windows */
    {
	int  err;

	/*
	 *  Skip the rest of this iteration if we didn't
	 *  start this subwindow.
	 */
	if ((use_start_flag && !(pRW->sw_flag & DMA_START_SW))
							||
	    pRW->sw_addr == NULL)
		continue;

	/*
	 *  We don't need to set up addressibility here because
	 *  it is said that we only need the offset in the 3rd
	 *  argument.  Addressibility is obtained by segment info
	 *  in the cross memory attachment structure.
	 */
        err = d_complete ((int) pDS->dma_chan_id,
		          pRCM->flags & (DMA_READ | DMA_WRITE_ONLY),
		          pRW->sw_addr,		/* uses offset only */
		          (size_t) pRW->sw_length,
			  pXM,			/* has seg info */
        		  (caddr_t) pDS->d_dma_area[i].bus_addr);

	/*
	 *  Register the first error, only.
	 */
	if (!*error)
	    *error = err;
        if(err) {
                TRCHKGT(HKWD_DISPLAY_RESERVED_51D | __LINE__,
                    err, pRW->sw_addr, pRW->sw_length,
                    pRCM->flags & (DMA_READ | DMA_WRITE_ONLY),
                    getpid());
    }
    }

    /*
     *  Turn off state for device and our process.
     *  This should be efficient if we are already at interrupt level.
     */
    RCM_ASSERT (pproc->procHead.flags & PROC_DMA_IN_PROGRESS, 0, 0, 0, 0, 0);

    release_lock (pdev, pproc);
}


/*
 *  dma_end_slave - Slave window DMA termination code.
 *
 *  The only error that can be reported is the d_complete return code.
 *  It is stored in the struct miscel.error member.
 */
static void  dma_end_slave (pproc)
struct _rcmProc *pproc;
{
    int  *error;
    struct _miscel  *pmiscel;
    int     i;			/* current window number */
    gscDev *pdev;               /* pointer to gsc device structure    */
    struct _gscdma  *pRCM;	/* point to permanent DMA window info */
    struct sw  *pRW;		/* point to subwindow structure */
    struct xmem  *pXM;		/* point to cross mem attach structure */
 
    pRCM     = &pproc->procHead.gscdma;		/* kernel dma record */
    pmiscel  = (struct _miscel *) pRCM->error;	/* miscel struct ptr */
    error    = &pmiscel->error;		/* place to store error */
    i        = pmiscel->cur_win;	/* current window in process */

    pmiscel->seq++;			/* update callback sequence */

    pdev = pproc->procHead.pGSC;
    pRW  = &pRCM->subwindow[i];		/* sw info for i'th window */
    pXM  = &pproc->procHead.xmemdma[i];

    RCM_TRACE (0x729, getpid (), pdev, pproc);

    /*
     *  We don't need to set up addressibility here because
     *  it is said that we only need the offset in the 3rd
     *  argument.  Addressibility is obtained by segment info
     *  in the cross memory attachment structure.
     */
    *error = d_complete ((int) pdev->devHead.display->dma_chan_id,
		         pRCM->flags & (DMA_READ | DMA_WRITE_ONLY),
		         pRW->sw_addr,	/* uses offset only */
		         (size_t) pRW->sw_length,
			 pXM,		/* seg info from here */
		         NULL);
 
    /*
     *  Turn off state for device and our process, and wake up waiters.
     */
    RCM_ASSERT (pproc->procHead.flags & PROC_DMA_IN_PROGRESS, 0, 0, 0, 0, 0);

    release_lock (pdev, pproc);
}


/*
 *  take_down_sw - Take down resource assignments for a subwindow.
 *
 *  The resources controlled are:
 *
 *  1)  cross memory attachment.
 *  2)  pinning of user space.
 *
 *  No error is returned.  Resource takedown error causes assert.
 *
 *  WARNING:  It is critical that the xmdetach be done AFTER the
 *  unpin operation.  The whole kernel will retain our access to
 *  the segment, even if it's shared memory that users have detached
 *  from, as long as we have our xmattach in force when we try to unpin.
 */
static int  take_down_sw (pRW, pXM, pSG)
struct sw  *pRW;
struct xmem *pXM; /*  = &pproc->procHead.xmemdma[i] */
struct _gscdma_ext *pSG;
{
    int  rc;

    /*
     *  Unpin the old window, if valid.  Always clear pin record.
     *
     *  To do this successfully requires that set up our own segment
     *  register.
     *
     *  See unpinu () in com/sys/ios/uio.c for the model for this.
     *  We assume the buffer doesn't span segments.  Note that we call
     *  unpin, not unpinu.  This is apparently due to the fact that the
     *  segment register we allocate and use is in the system set, not
     *  the user set, as is assumed by unpinu.
     */
    if (pRW->sw_addr != NULL)
    {
	caddr_t  newbase;

	newbase = vm_att (pSG->sw_seg, pRW->sw_addr);

	CALLC (unpin (newbase, pRW->sw_length));

	vm_det (newbase);
    }

    pRW->sw_addr   = NULL;
    pSG->sw_seg    = NULL;
    pRW->sw_length = 0   ;
    pRW->sw_flag   = 0   ;

    /*
     *  Detach any valid cross mem attachment.  Invalidate
     *  the attachment record.
     */
    if (pXM->aspace_id != XMEM_INVAL)
        CALLC (xmdetach (pXM));

    pXM->aspace_id = XMEM_INVAL;

    return  0;
}


/*
 *  set_up_sw - Set up resource assignments for a subwindow.
 *
 *  The resources controlled are:
 *
 *  1)  pinning of user space.
 *  2)  cross memory attachment.
 *
 *  The only errors that can be returned are pinning or cross memory
 *  attachment errors.
 */
static int  set_up_sw (pUW, pRW, pXM, pSG)
struct sw    *pUW;	/* point to window structure in pUSR */
struct sw    *pRW;	/* point to window structure in pRCM */
struct xmem  *pXM; 	/* &pproc->procHead.xmemdma[i] */
struct _gscdma_ext  *pSG;
{
    int  rc;
    int  c, d;
    int  write_protected = 0;

    /*
     *  Mark all resources as "unallocated".
     *
     *  The ones that get allocated on this call will be so marked that
     *  the takedown routine will be successful in removing exactly what's
     *  actually allocated.
     */
    pRW->sw_addr   = NULL;
    pXM->aspace_id = XMEM_INVAL;

    /*
     *  Validate storage addresses for read/write.
     *
     *  Fetch bytes from the buffer.  It should be readable.
     *  Then, write them back and remember whether it's writable.
     *
     *  This is not a thorough check.  There ought to be a better
     *  way to get the addressability and length of the alleged
     *  buffer without doing this.
     */
    c = fubyte (pUW->sw_addr);
    d = fubyte (pUW->sw_addr+pUW->sw_length-1);

    if (c == -1 || d == -1)
	return EACCES;

    c = subyte (pUW->sw_addr, c);
    d = subyte (pUW->sw_addr+pUW->sw_length-1, d);

    if (c || d)
	write_protected = 1;

    /*
     *  Attempt to perform cross mem attachment for DMA.  Success
     *  automatically sets the validity flag for later detach calls.
     *  Possible error codes are OK for user.  If successful, this sets
     *  the validity flag.
     *
     *  Successfully performing this attachment also has the effect of
     *  rendering this piece of memory permanently accessible to us
     *  (for a later unpin) until we perform the cross memory detach.
     *  The only difficulty will be addressing the piece of memory, since
     *  the user's seg reg setup may be different at unpin (and d_complete
     *  time).
     */
    CALLR (xmattach (pUW->sw_addr, pUW->sw_length, pXM, USER_ADSPACE));

    /*
     *  Attempt to pin the user space.  Possible error codes are OK for user.
     *
     *  This uses pinu to get access to the user segments.  They are presumed
     *  to be set up rightly, since we have just come in on the system call.
     */
    CALLR (pinu (pUW->sw_addr, pUW->sw_length, UIO_USERSPACE));

    /*
     *  Mark success for later unpins.
     */
    pRW->sw_addr   = pUW->sw_addr  ;
    pSG->sw_seg    = pSG->sw_Useg;	/* we MUST be called from alter sw */
    pRW->sw_length = pUW->sw_length;
    pRW->sw_flag   = pUW->sw_flag  ;

    /* force the flag setting to avoid contamination from user parms */
    if (write_protected)
	pRW->sw_flag |=  DMA_SW_WRITE_PROTECT;
    else
	pRW->sw_flag &= ~DMA_SW_WRITE_PROTECT;

    return 0;
}


/*
 *  alter_subwindow - Test to see if the current subwindow's dma configuration
 *		      has been changed by the user on this call.  If so, take
 *		      down the current and set up the new.
 *
 *  The only errors tolerated are setup errors.
 */
static int alter_subwindow (pUW, pRW, pXM, pSG, read)
struct sw    *pUW;	/* point to window structure in pUSR */
struct sw    *pRW;	/* point to window structure in pRCM */
struct xmem  *pXM; 	/* &pproc->procHead.xmemdma[i] */
struct _gscdma_ext  *pSG;
int  read;		/* set if dma read from device into host mem */
{
    int  rc;

    /*
     *  If the old and new subwindow origins don't match, or if the
     *  length of a new actual subwindow is greater than what has
     *  been already pinned, then release subwindow resources and
     *  reacquire them for the new conditions.
     *
     *  Address comparison MUST include the content of the appropriate
     *  segment register!  This is because the offset might stay the
     *  same, but the vmhandle might change!
     */
    rc = 0;

    pSG->sw_Useg = NULL;		/* pRW->sw_Useg is for user arg! */
    if (pUW->sw_addr != NULL)
        pSG->sw_Useg  = as_getsrval (getadsp (), pUW->sw_addr);

    /*
     *  To see if we need to change the subwindow, compare the effective
     *  address of the previous and current request.
     *
     *  Note: Our simple code to do this ALSO checks that the same segment
     *  register is used, and that the segment register has the same access
     *  control bits in its upper part.  This may not really be necessary,
     *  but it IS safe!  I don't think it can hurt performance.
     */
    if (pSG->sw_Useg != pSG->sw_seg  ||
	pUW->sw_addr != pRW->sw_addr ||
	(pRW->sw_addr != NULL && pRW->sw_length != pUW->sw_length))
    {
        if (pRW->sw_addr != NULL)	/* take down the old */
            take_down_sw (pRW, pXM, pSG);

        if (pUW->sw_addr != NULL)	/* set up the new */
            rc = set_up_sw (pUW, pRW, pXM, pSG);

	if (!rc                                     &&    /* no prev error */
	    read                                    &&    /* read into buffer */
	    (pRW->sw_flag &  DMA_SW_WRITE_PROTECT)      ) /* which is prot'd */
	{
	    rc = EACCES;			/* give this error */
	}

#ifdef RCMDEBUGGSCDMA
	/*
	 *  If reading into host memory and if not PowerPC architecture,
	 *  then make check for buffer alignment which can ALLOW cache
	 *  interference problems with the processor.  (It doesn't mean
	 *  there is a real problem.)  The kind of alignment that allows
	 *  a problem is one where the buffer is not the sole occupant of
	 *  a sequence of one or more memory granules of a certain size.
	 *  This size is the maximum of the data cache line size and the
	 *  IOCC DMA buffer footprint size.  The RS1 IOCC DMA master data
	 *  buffers are 64 bytes in size (unless it is XIO, in which case
	 *  there is a second read-ahead buffer).  It is assumed that RS2
	 *  is the same as RS1.  This calculation is made even if the DMA
	 *  master is unbuffered, or if it is a DMA slave.
	 *
	 *  PowerPC doesn't have the problem because caches are coherent
	 *  throughout the system.
	 *
	 *  Is the configuration data available indicating whether we
	 *  have full-snoop enabled for cache coherency or not?  We are
	 *  going by what we know about the architecture right now.
	 *
	 *  Also, it is being assumed that the cache line sizes in the
	 *  configuration data structure are measured in bytes.
	 */

#define RCM_DMA_MASTER_BUF_SIZE		64	/* bytes */

	if (read && ! __power_pc ())
	{
/**/	    int  linesize;		/* d cache line size (in bytes) */

	    /* get max of dcache and IOCC buffer sizes */
	    linesize = _system_configuration.dcache_line;
	    if (linesize < RCM_DMA_MASTER_BUF_SIZE)
		linesize = RCM_DMA_MASTER_BUF_SIZE;

	    /*
	     *  Warn if not absolutely secure.  Of course, sw_length can
	     *  really be anything.  For this test we would need to know
	     *  whether the memory granules containing ANY part of the I/O
	     *  buffer also contain any other data that can be touched by the
	     *  processor.  This test just assumes that if the buffer
	     *  boundaries are aligned, there CAN'T be any other data.
	     *
	     *  Also, this cannot take into account the 64 byte read-ahead
	     *  buffer in XIO type IOCC's.
	     */
	    if ( ( (ulong) pUW->sw_addr   % linesize) ||
		 (         pUW->sw_length % linesize)    )
	    {
		printf (
  "gscdma: input buf 0x%x (len 0x%x) not aligned w/ dcache lines (size 0x%x)\n",
				pUW->sw_addr, pUW->sw_length, linesize);
	    }
	}
#endif
    }

    return  rc;
}


/*
 *  wait_lock - Wait for any DMA to conclude and acquire the DMA lock
 *		for ourselves.
 *
 *  This is only for use by process level code.
 */
static void  wait_lock (pdev, pproc)
gscDev *pdev;               /* pointer to gsc device structure    */
struct _rcmProc *pproc;
{
    int  old_int;

    old_int = i_disable (INTMAX);

    while (pdev->devHead.flags & DEV_DMA_IN_PROGRESS)
	e_sleep (&pdev->devHead.dma_sleep, EVENT_SHORT);

    pproc->procHead.flags |= PROC_DMA_IN_PROGRESS;
    pdev->devHead.flags   |=  DEV_DMA_IN_PROGRESS;

    i_enable (old_int);
}


/*
 *  wait_lock_if_ours - Wait for conclusion of our process DMA and
 *			reacquire the DMA lock for ourselves.
 *			Also, acquire it if no DMA at all is active.
 *			If DMA in progress is NOT ours, do not wait or
 *			acquire the lock!
 *
 *  This is only for use by process level code.
 */
static void  wait_lock_if_ours (pdev, pproc)
gscDev *pdev;               /* pointer to gsc device structure    */
struct _rcmProc *pproc;
{
    int  old_int;

    old_int = i_disable (INTMAX);

    /*
     *  If our DMA is in progress, wait till dma concludes on the
     *  device and reacquire the lock.  Do likewise if NO-ONE's
     *  DMA is active.
     */
    if (pproc->procHead.flags & PROC_DMA_IN_PROGRESS || 
        !(pdev->devHead.flags & DEV_DMA_IN_PROGRESS)    )
    {
        while (pdev->devHead.flags & DEV_DMA_IN_PROGRESS)
	    e_sleep (&pdev->devHead.dma_sleep, EVENT_SHORT);

        pproc->procHead.flags |= PROC_DMA_IN_PROGRESS;
        pdev->devHead.flags   |=  DEV_DMA_IN_PROGRESS;
    }

    i_enable (old_int);
}


/*
 *  wait_end_dma - Wait for our process DMA to conclude.  Do NOT acquire
 *		   the lock.
 *
 *  This is only for use by process level code.
 */
static void  wait_end_dma (pdev, pproc)
gscDev *pdev;               /* pointer to gsc device structure    */
struct _rcmProc *pproc;
{
    int  old_int;

    if (pproc->procHead.flags & PROC_DMA_IN_PROGRESS)
    {
	/*
	 *  Wait for our process DMA, only, to complete.
	 *  Ignore anyone else's.
	 */
        old_int = i_disable (INTMAX);

        while (pproc->procHead.flags & PROC_DMA_IN_PROGRESS)
	    e_sleep (&pdev->devHead.dma_sleep, EVENT_SHORT);

        i_enable (old_int);
    }
}


/*
 *  release_lock - Release DMA use lock, if we have it.
 *
 *  This is only for use by process level code.
 */
static void  release_lock (pdev, pproc)
gscDev *pdev;               /* pointer to gsc device structure    */
struct _rcmProc *pproc;
{
    int  old_int;

    if (pproc->procHead.flags & PROC_DMA_IN_PROGRESS)
    {
        old_int = i_disable (INTMAX);

        pproc->procHead.flags &= ~PROC_DMA_IN_PROGRESS;
        pdev->devHead.flags   &=  ~DEV_DMA_IN_PROGRESS;

        if (pdev->devHead.dma_sleep != EVENT_NULL)
	    e_wakeup (&pdev->devHead.dma_sleep);

        i_enable (old_int);
    }
}


/*------------
 
  dma_service
 
  This service assumes the following:
 
  1) at init time, the display device driver has issued a d_init()
     call to allocate and initialize a DMA channel, and that
     the assigned DMA channel is stored in its dds.
 
  2) Buffer(s) in user space have been allocated for the transfer.
     If the transfer is system to adapter, the buffer(s) contain
     the data to be transferred.
 
  ------------*/
 
int  dma_service (pd, arg)
struct phys_displays *pd;       /* phys display structure pointer     */
struct _gscdma *arg;            /* pointer to a gscbuf struct         */
{
    gscDev *pdev;               /* pointer to gsc device structure    */
    struct _rcmProc *pproc;     /* pointer to rcm process structure   */
    int  i, rc, adapter_lock = 0;

    SET_PDEV (pd, pdev);
    ASSERT (pdev != NULL);

    FIND_GP (pdev, pproc);
    if (pproc == NULL)
	return (EINVAL);
 
    RCM_TRACE (0x700, getpid (), pdev, pproc);

    /*
     *  If this adapter makes callbacks to guard the domain on this thread
     *  then it has to have set the DEV_DMA_LOCK_ADAPTER flag at DDdev_init
     *  time.  So far, this is only the PED/LEGA family.
     */
    if (pdev->devHead.flags & DEV_DMA_LOCK_ADAPTER)
    {
	/*
	 *  Perform conditional adapter lock for ped world.
	 *
	 *  No critical sequence protection required, since our values are
	 *  either present and static, or pLockProc will not be us, even if
	 *  it's not static.
	 */
	 if (!  ( (pdev->devHead.flags & DEV_GP_LOCKED_AND_GUARDED) &&
		   pdev->devHead.pLockProc == pproc                    )
		)
	{
	    /*
	     *  We do not have the adapter locked ourselves.
	     *
	     *  We must also check to make sure we have no domain guarded by
	     *  system call.  This must be done because adapter lock and
	     *  system call domain lock are not orthogonally handled for the
	     *  same thread in the guard_dom/unguard_dom functions!  If one
	     *  style is in effect, then the other cannot be (for the same
	     *  thread).
	     *
	     *  No critical section protection is required, since the critical
	     *  values will be ours and static, or will not be ours.
	     */
	    for (i=0; i<pdev->devHead.num_domains; i++)
	    {
		if ((pdev->domain[i].flags & DOMAIN_LOCKED) &&
		     pdev->domain[i].pLockProc == pproc         )
		{
		    break;
		}
	    }

	    /*
	     *  If we have no syscall domain locks, either, then we have no
	     *  syscall locks set at all.  We lock the adapter.  Else, we
	     *  assume some kind of locking is already in effect and do nothing.
	     */
	    if (i == pdev->devHead.num_domains)
	    {
		rcm_lock_pdev (pdev, pproc, PDEV_GUARD);

		adapter_lock = 1;
	    }
	}
    }

    rc = dma_service1 (pd, pdev, pproc, arg);

    if (adapter_lock)
	rcm_unlock_pdev (pdev, pproc, 0);

    return  rc;
}


int  dma_service1 (pd, pdev, pproc, arg)
struct phys_displays *pd;       /* phys display structure pointer     */
gscDev *pdev;                   /* pointer to gsc device structure    */
struct _rcmProc *pproc;         /* pointer to rcm process structure   */
struct _gscdma *arg;            /* pointer to a gscbuf struct         */
{
    struct _gscdma  usrdma;	/* temp stack dma argument structure  */
    int  dma_flags;             /* parameter to d_master service      */
    int  i;                     /* index                              */
    int rc;                     /* return code                        */
    int  seq;			/* hold callback seq number	      */
    struct _gscdma  *pRCM;	/* point to permanent DMA window info */
    struct sw       *pRW;
    struct _gscdma  *pUSR;	/* point to user syscall DMA wndw info*/
    struct sw       *pUW;
    struct xmem     *pXM;
    struct _gscdma_ext  *pSG;
    struct _miscel  *pmiscel;
    struct phys_displays  *pDS;
    int  error, diagmode;
    long  (*f_diag) (), (*f_setup) (), (*f_go) ();
    int  master, slave;
    ulong use_start_flag;
    int  read;

    pDS = pdev->devHead.display;

    /*
     *  The _gscdma structure in the rcm process structure is used by the
     *  rcm to cache permanent information about pinned memory pages, etc.
     *  When DMA device dependent code is called, this structure contains
     *  the necessary information from the user system call.  Before the
     *  previous DMA operation initiated by us is concluded, we keep away
     *  from this data structure.  A temporary structure is available on
     *  the stack.
     *
     *  It is assumed that we can retain sufficient window pinning information
     *  in this data structure.
     *
     *  NOTE:  make_gp MUST initialize the pRCM->sw_addr members to NULL.
     *  NOTE:  make_gp MUST initialize the pRCM->error   member to NULL.
     *  This is done at present by bzero operations in gsc_make_gp ().
     */
    slave = pdev->devHead.display->dma_characteristics & DMA_SLAVE_DEV;
    master = !slave;

    pRCM = &pproc->procHead.gscdma;		/* kernel dma record */
    pUSR = &usrdma;				/* stack dma request */

    /*
     *  Make sure per-process miscellaneous structure is initialized.
     *  This structure is only free'd by the dma_free function.
     */
    pmiscel = (struct _miscel *) pRCM->error;	/* kernel DMA Cmd buffer */

    if (pmiscel == NULL)			/* allocate structure */
    {
	pRCM->error = (int) xmalloc (sizeof (struct _miscel), 2, pinned_heap);

	if (pRCM->error == (int) NULL)
	    return ENOMEM;

	pmiscel = (struct _miscel *) pRCM->error;

	pmiscel->dma_cmd = NULL;			/* initialize */
	pmiscel->cmd_length = 0;
	pmiscel->error = 0;			/* slave intrpt error code */
	pmiscel->seq = 0;			/* callback sequence */
    }

    /*
     *  Copy user's _gscdma structure into stack space.
     */
    CALLRC (copyin (arg, pUSR, sizeof (struct _gscdma)), EFAULT);

    /*
     *  If the caller gave a DMA cmd string, then copy it into the per-
     *  -process dma command buffer.  This buffer never contracts during
     *  the lifetime of the process.  It is free'd by the dma_free function.
     */
    if (pUSR->dma_cmd != NULL)
    {
	/*
	 *  If the current permanent DMA cmd string buffer is too short
	 *  (or doesn't exist), allocate one long enough.
	 */
	if (pmiscel->cmd_length < pUSR->cmd_length)
	{
	    if (pmiscel->dma_cmd != NULL)
		xmfree (pmiscel->dma_cmd, pinned_heap);
	    pmiscel->dma_cmd = NULL;  /* in case error follows */

            pmiscel->dma_cmd = xmalloc (pUSR->cmd_length, 2, pinned_heap);
            if (pmiscel->dma_cmd == NULL)
		return ENOMEM;
	    pmiscel->cmd_length = pUSR->cmd_length;
	}

	/*
	 *  The DMA cmd string will (now) fit in the permanent buffer.
	 */
        CALLRC (copyin (pUSR->dma_cmd, pmiscel->dma_cmd, pUSR->cmd_length),
									EFAULT);
	pUSR->dma_cmd = pmiscel->dma_cmd;
    }
 
    /*
     *  Has the application polled for DMA completion?
     */
    if (pUSR->flags & DMA_POLL)
    {
	/*
	 *  Check the in-progress flag for the process.
	 */
	if (pproc->procHead.flags & PROC_DMA_IN_PROGRESS)
            error = DMA_INCOMPLETE;
        else
            error = 0;

	/*
	 *  Return only the error word.  Don't overwrite his
	 *  whole argument structure.
	 */
	CALLRC (suword (&arg->error, error), EFAULT);

        return 0;
    }
 
    /*
     *  Has the application requested DMA facilities free-up?
     *  NOTE:  dma_free will wait for any outstanding DMA initiated
     *  by this process to complete.  It will return any d_complete
     *  error in rc, for either master or slave.
     */
    if (pUSR->num_sw == 0 || pUSR->flags & DMA_FREE)
    {
        rc = dma_free (pdev, pproc);	/* free all resources */

	CALLRC (suword (&arg->error, rc), EFAULT);

	if (rc)				/* select STD errno code */
            return EIO;

        return 0;
    }

    /*
     *  Check for valid combinations of flags.
     */
    diagmode = pUSR->flags & DMA_DIAGNOSTICS;
    f_diag   = pDS->diag_svc;
    f_setup  = pDS->vttdma_setup;
    f_go     = pDS->vttdma;

    if (diagmode && f_diag == NULL)
         return EINVAL;
 
    /*
     *  Make sure the user called for READ/WRITE/RDWR.
     */
    if ((pUSR->flags & (DMA_READ | DMA_WRITE | DMA_RDWR)) == 0)
         return EINVAL;
 
/**//* is this nohide really proper.  is mem integrity preserved? */
    if (pUSR->flags & (DMA_READ | DMA_RDWR))
    {
	dma_flags = DMA_READ | DMA_NOHIDE;
	read = 1;
    }
    else
    {
	dma_flags = DMA_WRITE_ONLY;	/* This also does NOHIDE */
	read = 0;
    }

    pUSR->flags |= dma_flags;

    /*
     * Check for max number of subwindows exceeded.
     */
    if (pUSR->num_sw > MAX_SUBAREAS)
	return EINVAL;

    /*
     *  If DMA on the device is not active, or if it's ours, wait
     *  for any DMA in progress (ours), and acquire the lock.
     *  If DMA is in progress for another process using this device,
     *  DO NOT acquire the lock.  This absurd complexity is for
     *  optimization purposes only, since the only thing we REALLY
     *  need to wait for here is for PROC_DMA_IN_PROGRESS to go away!
     *  This avoids calling the kernel again down below when we MUST
     *  acquire the DMA lock.
     */
    wait_lock_if_ours (pdev, pproc);

    /*
     *  Now transfer information from the stack gscdma structure
     *  to the kernel space version.
     */
    pRCM->flags      = pUSR->flags;
    pRCM->dma_cmd    = pUSR->dma_cmd;
    pRCM->cmd_length = pUSR->cmd_length;
    pRCM->num_sw     = pUSR->num_sw;

    use_start_flag = pRCM->flags & DMA_START_FLAG;

    /*
     *  Separate logic from here on out into master vs. slave.
     */
    if (master)
    {
        /*
         *  Perform DMA with device master.
	 *
	 *  Set up all subwindows before starting.  The DMA master device
	 *  processes all DMA in all subwindows before interrupting.
         */

        /*
	 *  Handle window setup for those subwindows to be used in this
	 *  DMA session.
	 *
	 *  IF THE SUBWINDOW CONFIGURATION IS NOT ALTERED FROM THE PREVIOUS
	 *  DMA CALL, THEN THIS SUBWINDOW LOOP DOES NOTHING.  THIS IS THE
	 *  OPTIMUM MODE OF CONTINUED USAGE.
         */
        rc = 0;					/* for d_complete */
        for (i=0, pUW=pUSR->subwindow,		/* slide down subwindows */
	          pRW=pRCM->subwindow,
		  pXM=pproc->procHead.xmemdma,
		  pSG=pproc->procHead.gscdma_ext;
             i<pRCM->num_sw;
             i++, pUW++, pRW++, pXM++,pSG++)
        {
	    /*
	     *  Skip the rest of this iteration if we aren't to start this
	     *  subwindow.
	     */
	    if ((use_start_flag && !(pUW->sw_flag & DMA_START_SW)) ||
		pUW->sw_addr == NULL)
	        continue;

	    /*
	     *  Munge the subwindow setup ONLY IF CHANGES ARE REQUIRED.
	     */
	    if (rc = alter_subwindow (pUW, pRW, pXM, pSG, read))
	    {
		release_lock (pdev, pproc);	/* if acquired */
		(void) dma_free (pdev, pproc);	/* free resources */
		return  rc;
	    }
	}

        /*
	 *  If we did not acquire the lock previously (due to someone
	 *  else's DMA being in progress) then wait for DMA to finish
	 *  and acquire it now.
	 *
	 *  Ignore signals when waiting.
         */
        if (!(pproc->procHead.flags & PROC_DMA_IN_PROGRESS))
	    wait_lock (pdev, pproc);
 
	/*
	 *  Check error code from last master termination.
	 *
	 *  This will be from one of the subwindows started on the last
	 *  dma_service call.
	 */
	error = pmiscel->error;

	if (!error)
	{
	    /*
	     *  Start the adapter setup and return while it's getting
	     *  ready.
	     */
	    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
	    if (!diagmode && f_setup != NULL)
	        error = (*f_setup) (pdev, pRCM);
	    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);

	    /*
	     *  Perform d_master on all subwindows.
	     */
	    if (!error)
	    {
	        for (i=0,
	             pRW=pRCM->subwindow,
	             pXM=pproc->procHead.xmemdma;
	             i<pRCM->num_sw;
	             i++,pRW++,pXM++)
	        {
	            /*
	             *  Skip the rest of this iteration if we aren't to
	             *  start this subwindow.
	             */
	            if ((use_start_flag && !(pRW->sw_flag & DMA_START_SW)) ||
		        pRW->sw_addr == NULL)
		        continue;

		    RCM_ASSERT (pXM->aspace_id == XMEM_PROC, 0, 0, 0, 0, 0);

	            d_master ((int) pDS->dma_chan_id,
		              pRCM->flags & (DMA_READ | DMA_WRITE_ONLY),
		              pRW->sw_addr,
		              (size_t) pRW->sw_length,
		              pXM,
			      (caddr_t) pDS->d_dma_area[i].bus_addr);
		    /*  It is said that .bus_addr doesn't need to have
			the offset in the memory page of the first byte
			of the transfer buffer added to it. */
	        }

		/*
		 *  Rule for vttdma:  Don't call the interrupt routine
		 *  if an error is returned in-line.  The d_master and
		 *  d_complete balance will be upset.
		 */
		seq = pmiscel->seq;

	        RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
	        if (diagmode)
		    error = (*f_diag) (pdev, pRCM, dma_end_master);
	        else
		    error = (*f_go  ) (pdev, pRCM, dma_end_master,
							pd->visible_vt, 0);
	        RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);

		/*
		 *  If we had an error and the device layer didn't call
		 *  the callback, then do it now.  WE ASSUME THAT NO
		 *  ASYNC DMA (WHICH WILL CALL THE CALLBACK LATER) IS
		 *  CURRENTLY ACTIVE.
		 */
		if (error && seq == pmiscel->seq)
		    dma_end_master (pproc);

		/*
		 *  If DMA_WAIT, then the callback has already been called
		 *  and we can pick up any error and report it now, instead
		 *  of on the next call.
		 *
		 *  If we do report an error, the facilities will be released
		 *  below.  This has the vital side effect of causing the
		 *  error to be forgotten by the next dma_service call by
		 *  this process.
		 */
		if (pUSR->flags & DMA_WAIT)
		    error = pmiscel->error;
	    }
	}

	/*
	 *  Handle any error.  This may be from the last dma operation
	 *  or from an in-line error occuring during the DD calls.
	 */
	if (error)
        {
	    release_lock (pdev, pproc);		/* in case still held */
	    /* releases pmiscel->error, et al */
	    (void) dma_free (pdev, pproc);
	    CALLRC (suword (&arg->error, error), EFAULT);
            return EIO;
        }

        return 0;

    } /* end master initiation */
    else
    /*
     *  Perform slave operations.
     *
     *  Slaves run each subwindow serially.
     */
    {
	/*
	 *  Slaves run each subwindow serially.
	 *
	 *  This code overlaps DMA on the current subwindow with resource
	 *  preparation (pin, etc.) for the next one.
	 */
        for (i=0, pUW=pUSR->subwindow,		/* slide down subwindows */
		  pRW=pRCM->subwindow,
	          pXM=pproc->procHead.xmemdma,
		  pSG=pproc->procHead.gscdma_ext;
             i<pRCM->num_sw;
             i++, pUW++, pRW++, pXM++,pSG++)
        {
	    /*
	     *  Skip the rest of this iteration if we aren't to start this
	     *  subwindow.
	     */
	    if ((use_start_flag && !(pUW->sw_flag & DMA_START_SW)) ||
		pUW->sw_addr == NULL)
	        continue;

	    /*
	     *  Munge the subwindow setup ONLY IF CHANGES ARE REQUIRED.
	     */
	    if (rc = alter_subwindow (pUW, pRW, pXM, pSG, read))
	    {
		release_lock (pdev, pproc);	/* if acquired */
		(void) dma_free (pdev, pproc);	/* free resources */
		return rc;
	    }

    	    /*
	     *  We are now ready to start the current subwindow.
	     *  We must check for completion of our last process
	     *  dma action, and check the exit code for it.
	     *  We must then acquire exclusive use of the device
	     *  to initiate the next DMA.
	     *
	     *  Signals are ignored when waiting.
	     *
	     *  We might have already acquired the lock on the first
	     *  iteration.
    	     */
            if (i != 0 || !(pproc->procHead.flags & PROC_DMA_IN_PROGRESS))
	    	wait_lock (pdev, pproc);
 
	    /*
	     *  Check error code from last slave termination.
	     *
	     *  This may be from the last slave subwindow started in this
	     *  loop, or from the last one started in the last dma_service
	     *  call.
	     */
	    error = pmiscel->error;

	    /*
	     *  Start next operation, if no error on last one.
	     */
	    if (!error)
	    {
	        RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
	        if (!diagmode && f_setup != NULL)
		    error = (*f_setup) (pdev, pRCM);
	        RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);

		/*
		 *  Press on if no error.
		 */
		if (!error)
		{
	            /*
	             *  Acquire access to system DMA controller.
	             */
		    RCM_ASSERT (pXM->aspace_id == XMEM_PROC, 0, 0, 0, 0, 0);

                    d_slave ((int) pDS->dma_chan_id,
                             pRCM->flags & (DMA_READ | DMA_WRITE_ONLY),
                             pRW->sw_addr,
		             (size_t) pRW->sw_length,
                             pXM);

	            /*
	             *  Start the actual DMA operation.
		     *
		     *  Rule for vttdma:  Don't call the interrupt routine
		     *  if an error is returned in-line.  The
		     *  d_slave/d_complete balance will be upset.
	             */
		    seq = pmiscel->seq;
	            pmiscel->cur_win = i;

	    	    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
	            if (diagmode)
		        error = (*f_diag) (pdev, pRCM, dma_end_slave);
	            else
		        error = (*f_go)   (pdev, pRCM, dma_end_slave,
							pd->visible_vt, i);
	    	    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);

		    /*
		     *  If we had an error and the device layer didn't call
		     *  the callback, then do it now.  WE ASSUME THAT NO
		     *  ASYNC DMA (WHICH WILL CALL THE CALLBACK LATER) IS
		     *  CURRENTLY ACTIVE.
		     */
		    if (error && seq == pmiscel->seq)
			dma_end_slave (pproc);

		    /*
		     *  If DMA_WAIT, then the callback has already been called
		     *  and we can pick up any error and report it now, instead
		     *  of on the next call.
		     *
		     *  If we do report an error, the facilities will be re-
		     *  leased below.  This has the vital side effect of causing
		     *  the error to be forgotten by the next dma_service call
		     *  by this process.
		     */
		    if (pUSR->flags & DMA_WAIT)
			error = pmiscel->error;
		}
	    }

            /*
	     *  Handle error from last subwindow, OR from this one.
             */
            if (error)
            {
		release_lock (pdev, pproc);	/* in case still held */
		/* release pmiscel->error, et al */
	        (void) dma_free (pdev, pproc);
	        CALLRC (suword (&arg->error, error), EFAULT);
                return EIO;
            }
        } /* end initiate DMA loop */
 
        return 0;

    } /* end slave sequence */
}
