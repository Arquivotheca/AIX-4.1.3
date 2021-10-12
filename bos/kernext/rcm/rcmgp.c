static char sccsid[] = "@(#)38	1.64.3.31  src/bos/kernext/rcm/rcmgp.c, rcm, bos41J, 9520A_all 5/3/95 11:45:45";

/*
 *   COMPONENT_NAME: (rcm) Rendering Context Manager Make/Unmake Graphics Mode
 *
 *   FUNCTIONS: BUGVDEF
 *		com_init
 *		com_term
 *		cproc_init
 *		cproc_term
 *		dev_init
 *		dev_term
 *		fix_mstsave
 *		gp_state_change
 *		devno_state_change
 *		rcm_state_change
 *		gsc_make_gp
 *		gsc_set_gp_priority
 *		gsc_unmake_gp
 *		rcm_kill_proc
 *		rcm_make_pproc
 *		rcm_unmake_gp
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989-1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <lft.h>                    /* includes for all lft related data */
#include <sys/ioctl.h>          /* includes for all lft ioctl data */
#include <sys/lft_ioctl.h>          /* includes for all lft ioctl data */
#include <sys/malloc.h> 		/* memory allocation routines */
#include <sys/user.h>			/* user structure */
#include <sys/adspace.h>		/* address space stuff	*/
#include <sys/lockl.h>			/* lock stuff  */
#include <sys/sleep.h>
#include <sys/ioacc.h>
#include <sys/signal.h>
#include <sys/syspest.h>
#include <sys/watchdog.h>

#ifdef   _KERNSYS
#include <sys/machine.h>
#else
#define  _KERNSYS
#include <sys/machine.h>
#undef   _KERNSYS
#endif

#include "gscsubr.h"			/* functions */
#include "rcmdds.h"
#include "rcm_mac.h"
#include "rcmhsc.h"
#include "xmalloc_trace.h"
#include "rcm_pm.h"

/* external functions without .h files */
extern void  put_busmem_keys (gscComProcPtr, gscDevPtr);
extern int domain_acc_authority_init (gscComProcPtr, struct phys_displays *);
extern void rcm_usr_buffer_detach_all (gscComProc *);
extern update_pm_data();

/* global rcm DDS anchor point (in pinned code) */
rcm_dds *rcmdds = NULL;

/* internal functions */
static int com_init (), dev_init (), cproc_init ();
static void com_term (), dev_term (), cproc_term ();
static void rcm_kill_proc ();
static int rcm_make_pproc ();

static detach_seg_reg (rcmProcPtr pproc);
static detach_busprot (rcmProcPtr pproc);

BUGVDEF(dbg_rcmgp,99)	    /* for system call functions */
BUGVDEF(dbg_rcmgp_int,99)   /* for fault/interrupt level functions */
BUGVDEF(dbg_rcmgp_trace,99)   /* for trace prints */

struct _trace_all trace_all = { "RCMTRACE", 0 };

/*****************************************************************

		RCM COMMON ANCHOR

    The RCM common anchor provides a globally accessible anchor
    point for access to the RCM common structure.

    THIS STRUCTURE MUST BE PINNED.

******************************************************************/

comAnchor comAnc = {LOCK_AVAIL, NULL};

/******************************************************************
    Information for accessing the bus and the IOCC
 ******************************************************************/

/*
 *  Traditional Microchannel adapters use the fixed bus id 0x20.
 *  Others may define the value themselves.  See the code in rcm_make_pproc
 *  for the exact interface "definition" between the RCM and drivers.
 */
#define MICROCHANNEL_STD_BUID (unsigned) 0x02000000
#define MICROCHANNEL_SEG_REG  (unsigned) 0xC00C0000
			/* this says address i/o space, in user mode, */
			/* address check, address increment, */
			/* with upper 4 bits of bus memory address zero. */
			/* this is compatible with PowerPC IOCC */

/* Power/Power2/PowerPC shared definitions */
#define PWR_TBIT_MASK	0x80000000		/* For T=1 segments */
#define PWR_ADDR_MASK	0x0000000f		/* upper 4 bits of address */
#define PWR_IBIT_MASK	0x00000080		/* IOCC space select */

/* Power/Power2 masks for segment registers */
#define PWR_KBIT_MASK	0x40000000
#define PWR_BUSID_MASK	0x0ff00000
#define PWR_BBIT_MASK	0x00000020
#define PWR_MBIT_MASK	0x00000040		/* RT Compatibility mode */
#define PWR_ADIC_MASK	0x000c0000		/* Address incr/check bits */
#define PWR_RESV_MASK	0x3003ff10		/* Reserves (must be zero) */

/* PowerPC masks for segment registers */
#define PPC_KBIT_MASK	0x60000000
#define PPC_BUSID_MASK	0x1ff00000
#define PPC_RESV_MASK	0x000fff70		/* Reserves (must be zero) */

#define IOCC_NO_IO	0xFFFF0000	/* this allows no access to bus i/o */

/*
 *  The following macro is only used in the Power/Power2 architecture to
 *  turn the user space segment register value into a value usable from system
 *  space to access the IOCC control space.  The operating system bus per-
 *  missions software needs it to update control structures inside the IOCC.
 *
 *  All known authorization modes are turned OFF by clearing the K bit, and
 *  turning on the Bypass bit.  (This may be redundant, but works and can't
 *  hurt).  The address extension is cleared.  And, the IOCC address space
 *  bit is turned on.  Other bits in the segment register are untouched.
 */
#define CVT_IOCC(seg) (unsigned) (((seg) & ~(PWR_KBIT_MASK | PWR_ADDR_MASK)) | \
					    (PWR_IBIT_MASK | PWR_BBIT_MASK)    )

/* ============================================================= */
/* FUNCTION: gsc_make_gp
*/
/* PURPOSE: makes a process a graphics process
*/
/* DESCRIPTION:
	    Sets up "null" rendering contexts for all protection
	    domains in the device. This allows the graphics
	    process to address the entire device without restoring
	    a context; a non-null context will be saved, however.
*/
/* INVOCATION:
    int gsc_make_gp (pd, parg, parm1)
	struct phys_displays *pd;
	make_gp 	    *parg;
	int		    parm1;
*/
/* CALLS:
	rcm_init	- to initialize vt structure for the RCM
	vddmake_gp	- to return device information
*/
/* DATA:
	Accesses virtual terminal structure, DDS for adapter
*/
/* RETURNS:
	0 if successful
	ERRNO if unsuccessful
*/
/* OUTPUT:
	error code
	segment base
	device dependent structure containing device address offsets
	creates various global structures
*/
/* RELATED INFORMATION:
*/
/* NOTES:
	Sets up the RCM common structure, an RCM device structure,
	and a RCM common process structure when necessary. It always
	sets up a RCM device process structure (by calling rcm_make_
	pproc).

	Assumes that the TCWs have been set up

	If the adapter does not support DWA, then there will only be
	one "graphics process." That gp may wish to get interrupts
	for some reason, thus it is necessary to set up all the
	structures needed to handle interrupts.

	A process can request exclusive access to a device. The device
	is then treated like a non-DWA device. A process should only
	do this for one device at a time.

	The first graphics process in the list is considered the
	gp leader.  When this process terminates, all other graphics
	processes will be signalled (see "gp_leader" in rcm_unmake_gp).

*/

int gsc_make_gp (pd, parg, parm1)
    struct phys_displays *pd;
    make_gp		*parg;
    int			parm1;
{
    make_gp	    a;
    int 	    i;		    /* local */
    gscDevPtr	    pdev;
    gscComProcPtr   pcproc;
    rcmProcPtr	    pproc;	    /* pointer to rcm proc */
    int             lock_stat, old_int;
    int		    *prev_funl = (int *) parm1;

    gsctrace (MAKE_GP, PTID_ENTRY);
    /* get argument */
    if (copyin (parg, &a, sizeof (a))) {
	BUGPR(("###### make_gp ERROR copyin arg \n"));
	return (EFAULT);
    }

    BUGLPR(dbg_rcmgp,BUGNFO,
	   ("====== make_gp... pData=0x%x, len=%d\n", a.pData, a.length));

    /* check parms */
    if ((a.pData == NULL) || (a.length <= 0)) {
	BUGPR(("###### make_gp ERROR no data pointer/length \n"));
	return (EINVAL);
    }

    /* rcm lock common to prevent lists from corrosion */
    RCM_LOCK (&comAnc.rlock, lock_stat);

    /*
     *  Assure existence of common structure and shared memory attachment.
     */
    i = com_init (pd);			/* ensure existence of com struc */
    if (i)				/* if creation failed */
    {
	RCM_UNLOCK (&comAnc.rlock, lock_stat);
	BUGPR(("###### make_gp ERROR com_init\n"));
	return (i);
    }

    /*
     *  Assure existence of common process structure.
     *
     *  An error will be returned from cproc_init if cproc structure exists,
     *  and we are not the right thread.
     */
    FIND_COMPROC(pcproc);		/* find any cproc of this pid or null*/

    i = cproc_init (&pcproc, prev_funl);
    if (i)
    {
	com_term (pd);
	RCM_UNLOCK (&comAnc.rlock, lock_stat);
	BUGPR(("###### make_gp ERROR cproc_init\n"));
	return (i);
    }

    /*
     *  Assure existence of device structure for this VT.
     */
    i = dev_init (pd, pcproc);
    if (i)
    {
	cproc_term (pcproc, prev_funl);
	com_term (pd);
	RCM_UNLOCK (&comAnc.rlock, lock_stat);
	BUGPR(("###### make_gp ERROR dev_init\n"));
	return (i);
    }

    /* set the device pointer for further operations */
    SET_PDEV(pd,pdev);

    /* if the device is reserved, then return with error */
    if (pdev->devHead.flags & DEV_RESERVED) {
	dev_term (pd, pcproc);
	cproc_term (pcproc, prev_funl);
	com_term (pd);
	RCM_UNLOCK (&comAnc.rlock, lock_stat);
	BUGLPR(dbg_rcmgp, 0,
	       ("##### make_gp ERROR Device reserved; access denied.\n"));
	return (EACCES);
    }

    /* rcm unlock com */
    RCM_UNLOCK (&comAnc.rlock, lock_stat);

    /*
     *  Make rcm process structure (dev dependent) + all the
     *  null contexts that go with normal DWA handling.
     */
    i = rcm_make_pproc (pdev, pcproc, parg, &a, &pproc);
    if (i)
    {
	RCM_LOCK (&comAnc.rlock, lock_stat);
	dev_term (pd, pcproc);
	cproc_term (pcproc, prev_funl);
	com_term (pd);
        RCM_UNLOCK (&comAnc.rlock, lock_stat);
	return (i);
    }

    /*
     *  Pdev updates are serialized via rcm_(un)lock_pdev.  This is
     *  because the atomic update operation may (at some places) include
     *  more work that just the list update, and may not all appropriately
     *  be done with interrupts disabled.  Since this is required at at
     *  least one place in the rcm, ALL update operations to pdev which
     *  must serialize, must use the same locking mechanism.
     *
     *  We have an exception to this rule in 'create-win-geom', where
     *  the new entry is made in the hash list (under interrupt protection)
     *  but without locking pdev.  This is thought to be OK at this time,
     *  but it breaks the paradigm.  Perhaps the paradigm needs to be
     *  thoroughly reinvestigated to separate the different uses of the
     *  pdev lock.
     */
    rcm_lock_pdev (pdev, pproc, 0);

    /*
     *  Those who scan pdev (without updating it) do not use the update
     *  lock.  Scanner's simply inhibit interrupts.  Therefore, we must
     *  always inhibit interrupts around a pdev update operation which
     *  may be looked at by a list scanner.
     */
    old_int = i_disable(INTMAX);

    /*
     *  Link the process into the device structure.  If we are the first
     *  process this device has ever seen, then set the leader flags on
     *  the device and process.
     *
     *  As part of this unitary operation, check the DEV_SIGNAL_SENT flag
     *  to see if we have missed the termination signal given out by an
     *  unmaking leader.  If the flag is not set, then we proceed as usual.
     *  If the flag is set, then we have missed the SIGTERM.  We perform our
     *  own rcm_unmake_gp from right here.  We always return an error code
     *  (EINVAL) in this case.
     *
     *  The combination of SIGTERM to those on the process list, and in-line
     *  call from this point for late starters, should ensure that all rcm
     *  processes go down.  This is important, since the leader's unmake-gp
     *  sleeps for this condition.
     */

    /*  En-list  */
    pproc->procHead.pNext = pdev->devHead.pProc;
    pdev->devHead.pProc = pproc;

    /* set PROC_GP_LEADER, if never been a leader on this device */
    if (!(pdev->devHead.flags & DEV_HAS_LEADER))
    {
	pdev->devHead.flags |= DEV_HAS_LEADER;
	pproc->procHead.flags |= PROC_GP_LEADER;
    }

    /* If the leader has disabled the device, then undo it all. */
    if (pdev->devHead.flags & DEV_SIGNAL_SENT)
    {
	i_enable (old_int);
	rcm_unlock_pdev (pdev, pproc, 0);

	assert (rcm_unmake_gp (pdev, prev_funl) == 0);

	/*  all facilities release is done by rcm_unmake_gp */
	return  (EINVAL);
    }

    i_enable (old_int);
    rcm_unlock_pdev (pdev, pproc, 0);

    /*
     *  If we are the leader and haven't set up shared mem table ... 
     *
     *  Remember we are the leader on ALL heads of a multihead scenario
     *  and will come here for each head.
     */
    if (pproc->procHead.flags & PROC_GP_LEADER)
    {
	if (pproc->procHead.pCproc->pusrbufhdr == NULL)
	{
	    assert (pproc->procHead.pCproc->count == 1);

	    pproc->procHead.pCproc->pusrbufhdr =
		xmalloc (sizeof (struct usr_buffer *) * RCM_MAX_USR_BUFFER,
		3, pinned_heap);

	    /* if no mem, then to reverse this make requires unmake */
	    if (pproc->procHead.pCproc->pusrbufhdr == NULL)
	    {
		assert (rcm_unmake_gp (pdev, prev_funl) == 0);

		return (ENOMEM);
	    }

	    /* NULL out the usr_buffer pointer */
	    for (i=0; i<RCM_MAX_USR_BUFFER; i++)
		(pproc->procHead.pCproc->pusrbufhdr)[i] = NULL;
	}

	/* duplicate the pointer in pdev */
	pdev->devHead.pusrbufhdr = pproc->procHead.pCproc->pusrbufhdr;
    }

    RCM_TRACE(0x901,getpid(),pproc,pcproc);
    BUGLPR(dbg_rcmgp,BUGNFO,
	   ("==== Exit make_gp... pproc=0x%x\n\n", pproc));

    gsctrace (MAKE_GP, PTID_EXIT);
    return (0);
}


/*
 *  This function handles the creation of pproc and associated
 *  null contexts.
 */
static int rcm_make_pproc (pdev, pcproc, parg, pa, ppproc)
    gscDevPtr	    pdev;
    gscComProcPtr   pcproc;
    make_gp	    *parg, *pa;
    rcmProcPtr	    *ppproc;	    /* return the pproc pointer */
{
    char	    *pdd_data;	    /* device dependent data */
    rcmProcPtr	    pproc;	    /* pointer to rcm proc */
    int 	    i;		    /* local */
    int 	    proc_size;
    ulong	    old_int;
    ulong	    srval, srval_as_att;
    struct segreg  *segreg;
    ulong	    access_ctrl, busprot_type, dwa_device;
    ulong	    resource_id, bus_type;
    struct io_map  *pmap;
    struct phys_displays  *pd;

    /* if caller is already gp for device, return */
    FIND_GP(pdev,pproc);
    if (pproc != NULL) {
	BUGPR(("###### make_gp ERROR, already gp \n"));
	return (EINVAL);
    }

    /* get the input data */
    pdd_data = xmalloc (pa->length, 3, kernel_heap);
    if (pdd_data == NULL) {
	BUGPR(("###### make_gp ERROR xmalloc data \n"));
	return (ENOMEM);
    }

    if (copyin (pa->pData, pdd_data, pa->length)) {
	xmfree ((caddr_t) pdd_data, kernel_heap);
	BUGPR(("###### make_gp ERROR copyin data \n"));
	return (EFAULT);
    }

    /* allocate and initialize the RCM process structure */

    proc_size = sizeof (struct _rcmProc) +
	(pdev->devHead.num_domains - 1) * sizeof (char *);

    BUGLPR(dbg_rcmgp,BUGNFO,("====== create proc, size=0x%X \n", proc_size));

    pproc = xmalloc (proc_size, 3, pinned_heap);
    if (pproc == NULL) {
	xmfree ((caddr_t) pdd_data, kernel_heap);
	BUGPR(("###### make_gp ERROR xmalloc for proc, size=%d\n", i));
	return (ENOMEM);
    }

    *ppproc = pproc;			/* return the pointer */

    RCM_TRACE(0x900,getpid(),pproc,0);

    /* set kernel process id */
    pproc->procHead.pid = pcproc->pid;
    pproc->procHead.tid = pcproc->tid;

    BUGLPR(dbg_rcmgp,BUGNFO,
	   ("===== make_gp, pproc=0x%x, pid=0x%x\n",
	    pproc, pproc->procHead.pid));

    /* initialize other fields */
    pproc->procHead.pCproc = pcproc;
    pproc->procHead.priority = DEFAULT_PRIORITY;
    pproc->procHead.pRcx = NULL;
    pproc->procHead.pWA = NULL;
    pproc->procHead.flags = 0;
    pproc->procHead.pGSC = pdev;
    pproc->procHead.pPriv = NULL;
    pproc->procHead.srval = -1;
    pproc->procHead.pRcmbusprot = NULL;

    pproc->procHead.pEvents = NULL;

    bzero(&pproc->procHead.gscdma, sizeof(struct _gscdma));
    for (i = 0; i < MAX_SUBAREAS; i++)
    {
	bzero(&pproc->procHead.xmemdma[i], sizeof(struct xmem));
	bzero(&pproc->procHead.gscdma_ext[i], sizeof(struct _gscdma_ext));
    }

    pproc->procHead.pParts = NULL;

    /* if caller wants exclusive use of device */
    old_int = i_disable (INTMAX);
    if (pa->access & EXCLUSIVE_ACCESS)
    {
	/* if device not in use */
	if (pdev->devHead.pProc == NULL)
	    pdev->devHead.flags |= DEV_RESERVED;	/* reserve it */
	else
	{
	    BUGPR(("###### make_gp ERROR SHARE_ACCESS error\n"));
	    i_enable (old_int);
	    xmfree ((caddr_t) pdd_data, kernel_heap);
	    xmfree ((caddr_t) pproc, pinned_heap);
	    return (EACCES);				/* permission denied */
	}
    }
    i_enable (old_int);

    /* call vddmake_gp to get address information */
    BUGLPR(dbg_rcmgp,BUGNTA, ("====== calling dd make_gp\n"));
    /******************** WARNING *************************************/
    /* since cannot guard just one domain for make_gp, then do not    */
    /* try to guard any of them, this means that make_gp CANNOT do    */
    /* any context switches, except perhaps for the first make_gp     */
    /* for the system						      */
    /******************** WARNING *************************************/
    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
    pa->error = (pdev->devHead.display->make_gp)
					(pdev, pproc, pdd_data, pa->length,
					&trace_all);
    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
    if (pa->error) {
	BUGPR(("###### make_gp ERROR from dd make_gp, error=%d\n", pa->error));
	xmfree ((caddr_t) pproc, pinned_heap);
	xmfree ((caddr_t) pdd_data, kernel_heap);
	suword (&parg->error, pa->error);
	return (EIO);
    }

    /*  From this point on, use rcm_kill_proc to release pproc.
	This will release associated rcx's, call DD unmake_gp,
	release bus protection packets and segreg setups	*/

    /* return address information to process and free area */
    if (copyout (pdd_data, pa->pData, pa->length)) {
	BUGPR(("###### make_gp ERROR copying out address info\n"));
	rcm_kill_proc (pproc);
	xmfree ((caddr_t) pdd_data, kernel_heap);
	return (EFAULT);
    }
    xmfree ((caddr_t) pdd_data, kernel_heap);

    /*
     *  For all adapters it is necessary to set up a segment register to
     *  allow user code to have access to the permitted adapter domains.
     *
     *  Note:  The following discussion of "IOCC type adapters" refers to
     *  Microchannel adapters.  The Microchannel bus is the only bus currently
     *  attached to the IOCC.
     *
     *  For IOCC type adapters which share their 256MByte bus memory address
     *  space with other adapters, it is necessary to set up bus memory
     *  authorization key mechanisms to prevent access to unauthorized space.
     *
     *  The protection mechanisms are controlled in the Power/Power2 archi-
     *  tecture by programming TCW/TCE's associated with the IOCC memory spaces
     *  for the domains, and by setting bus protection keys in CSR15 of the
     *  IOCC.  This latter fact requires that bus protection control packets
     *  be registered with the kernel, so that the kernel can load the proper
     *  key bits into the CSR15 register of the IOCC whenever the process is
     *  resumed.  This is required, whether the adapter is a switching adapter
     *  or not.
     *
     *  In the PowerPC architecture, TCE's are programmed as above, but the bus
     *  memory protection keys are found in the segment register.
     *
     *  For IOCC type adapters which do not share their 256MByte bus memory
     *  address space with other adapters, it is not necessary to program
     *  TCW/TCE's, nor use any bus key settings in CSR15 or segment registers.
     *  In fact, TCW/TCE's may not exist for other bus memory segments than
     *  segment zero.  For these adapters, the segment register normally
     *  limits addressability sufficiently for system integrity.  Note, that
     *  adapters may still be context switching adapters.
     *
     *  TCW/TCE's are programmed by DDdev_init ().
     *
     *  The bus access protection mechanism provided by the kernel requires a
     *  registration packet per domain.  This is true for Power AND PowerPC.
     *
     *  NOTE:  Adapters which use bus I/O limits protection cannot be properly
     *  run on the PowerPC architecture, since the IOCC does not support limits
     *  registers or limits checking.  The rcm will reject such adapters.
     */

    /*
     *  The old graphics drivers which do not set up procHead.srval in the
     *  DDmake_gp function are based on the IOCC at the traditional bus id
     *  (0x20), and with segment value 0 (upper 4 bits of the bus memory
     *  address).  The supported adapter list includes:
     *
     *  1)  Skyway -	Power/Power2 architecture only, since bus I/O limits
     *			checking is required (not available on PowerPC).  This
     *			also requires kernel assist to get CSR15 set properly
     *			whenever the process using the adapter is resumed.
     *			This is a nonswitching adapter.  Warning:  There is
     *			code in the rcm to reject the make-gp call if any bus
     *			limits adapter is opened for graphics on a PowerPC
     *			platform.
     *
     *			Flag setup: access_ctrl  = 1; needs kernel setup
     *						       for Microchannel
     *				    dwa_device   = 0; needs nonswitch rcm setup
     *
     *  2)  Sabine -	Power/Power2 architecture only:  The Power Microchannel
     *			bus personalization register has been moved to a
     *			different place in the PowerPC address architecture,
     *			and the driver has not been modified.  (The Sabine
     *			driver modifies the dma arbitration time on the faster
     *			models.)  This is a switching adapter.  Warning:  Be-
     *			havior of this adapter on a PowerPC platform is un-
     *			defined and has not been tested.
     *
     *			Flag setup: access_ctrl  = 1; needs kernel setup
     *						       for Microchannel
     *				    dwa_device   = 1; needs switching rcm setup
     *
     *  3)  GTO -	Supported on all architectures.  This is a switching
     *			adapter.
     *
     *			Flag setup: access_ctrl  = 1; needs kernel setup
     *						       for Microchannel
     *				    dwa_device   = 1; needs switching rcm setup
     *
     *  4)  Ped/LEGA -	Supported on all architectures.  This is a family of
     *			switching and nonswitching adapters.  However, bus
     *			access support is provided to each member as if it
     *			were a switching adapter, because of the high/low
     *			water mark FIFO overflow control.
     *
     *			Flag setup: access_ctrl  = 1; needs kernel setup
     *						       for Microchannel
     *				    dwa_device   = 1; needs switching rcm setup
     *
     *  5)  Ruby -	Supported on all architectures.  This is a switching
     *			adapter.
     *
     *			Flag setup: access_ctrl  = 1; needs kernel setup
     *						       for Microchannel
     *				    dwa_device   = 1; needs switching rcm setup
     *
     *  The newer graphics drivers which DO set up procHead.srval in the
     *  DDmake_gp function are based on several busses.  The supported adapter
     *  list includes:
     *
     *  1)  SGA/WGA -	"Special graphics slot" adapters for the RSC version
     *			of the Power architecture (models 220/230).  These
     *			are not switching adapters and reside on their own
     *			private bus.  No kernel assistance for access control
     *			is required.
     *
     *			Flag setup: access_ctrl  = 0; needs no kernel setup
     *				    dwa_device   = 0; needs nonswitch rcm setup
     *
     *  2)  Baby Blue -	Processor bus adapter for the PowerPC architecture.
     *	      (601)	This is not a switching adapter, and does not share
     *			its 256MByte segment on that bus.  No kernel assistance
     *			for access control is required.
     *
     *			Flag setup: access_ctrl  = 0; needs no kernel setup
     *				    dwa_device   = 0; needs nonswitch rcm setup
     *
     *  3)  Baby Blue -	IOCC version for all architectures.  This is not a
     *	      (MCA)	switching adapter, and does not share its 256MByte
     *			segment on that bus.  No kernel assistance for access
     *			control is required.
     *
     *			Flag setup: access_ctrl  = 0; needs no kernel setup
     *				    dwa_device   = 0; needs nonswitch rcm setup
     *
     *  4)  Magenta   -	Processor bus adapter for the PowerPC architecture.
     *			This is a switching adapter.  On the 601 bus, access
     *			to the adapter is controlled via bus id 0x7F in a
     *			a segment register.  On 60X (not 601) busses, access
     *			is controlled via a data BAT (DBAT).  In all cases,
     *			the adapter is allocated the full 256 MBytes of space
     *			that is made accessible via the segment register on
     *			the 601 bus.  A different flavor of kernel assistance
     *			is used, depending on whether a segment register or
     *			a BAT is used to control access to the adapter.  The
     *			proper platform determination and associated setup is
     *			performed by the RCM.
     *
     *			Flag setup: access_ctrl  = 1; needs kernel setup
     *						       (set GS_BUS_AUTH_CONTROL)
     *				    dwa_device   = 1; needs switching rcm setup
     *
     *				    resource_id  = RID for xferdata
     */

#ifdef  U_IOMEM_ATT
    /*
     ************************************************************************
     *  Following is the u_iomem_att kloodge.  Hopefully, this can be more
     *  fully integrated into the RCM as time permits.
     *
     *  Currently, this is only used by the nonswitching RSPC adapters +
     *  Baby Blue on PowerPC.
     ************************************************************************
     */
    {
	struct io_map *p;

	p = (struct io_map *) pdev->devHead.display->io_map;

	if (p != NULL && !pdev->devHead.display->dwa_device)
	{
	    caddr_t  segment;

	    /*
	     *  Verify alignment and granularity.
	     *
	     *  Would be nice if kernel returned NULL from u_iomem_att
	     *  if checks fail, since it makes them, too.
	     */
	    if (p->size < MIN_BAT_SIZE         ||	/* minimum size */
		p->size > MAX_BAT_SIZE         || 	/* maximum size */
		((p->size - 1) & p->size) != 0 ||	/* power of 2 */
		p->busaddr % p->size != 0         )	/* aligned */
	    {
		rcm_kill_proc (pproc);
		return (EIO);
	    }

	    segment = u_iomem_att (p);

	    if (segment == NULL || suword (&parg->segment, segment))
	    {
		rcm_kill_proc (pproc);
		return (EFAULT);
	    }

	    pproc->procHead.pSegreg = (struct segreg *) segment;

	    for (i=0; i < pdev->devHead.num_domains; i++)
	    {
		pdev->domain[i].pCurProc = pproc;
		pdev->domain[i].pCur = NULL;
	    }

	    /* make sure we have a valid pproc pointer for DMA, events */
	    pdev->devHead.display->cur_rcm = pproc;

	    return (0);
	}
    }
#endif

    /*
     ************************************************************************
     *  Pick up all the protocol by which the driver tells us configurational
     *  things.
     ************************************************************************
     */

    /*
     *  Segment register may be specified by driver, or left alone, in which
     *  case the value here will still be as it was initialized:  -1.
     */
    srval = pproc->procHead.srval;

    /*
     *  The display->flags GS_BUS_AUTH_CONTROL flag bit indicates that
     *  kernel support will be necessary for access control.  Driver writers
     *  must be told how to set this bit by the rcm developer.  Generally,
     *  this bit means that an adapter needs the graphics fault mechanism
     *  because it is either a context switching adapter, or is an adapter
     *  which relies on bus access control to implement high/low water-mark.
     *  Or, this is an adapter which shares its bus with other devices and
     *  therefore needs bus access control to supply memory protection.
     *  If not explicitly set by the adapter, this bit will be OFF.
     */
    access_ctrl = (pdev->devHead.display->flags & GS_BUS_AUTH_CONTROL);

    /*
     *  The display->dwa_device flag indicates that the adapter requires
     *  the fault mechanism.  This is a proper subset of the meaning of
     *  the previous flag.  Adapters which switch contexts or which de-
     *  pend on the fault mechanism to implement high/low watermark must
     *  set this flag.
     */
    dwa_device = pdev->devHead.display->dwa_device;

    /*
     *  The display->ear word indicates the EAR register value that will
     *  be used by the adapter for eciwx/ecowx (xferdata) instructions.
     */
    resource_id = pdev->devHead.display->ear & EAR_DISABLE_MASK;

    /*
     *  The display->io_map pointer is used for BAT setup.
     */
    pmap = pdev->devHead.display->io_map;

    /*
     *  The display->bus_type field is used for bus selection.
     */
    bus_type = pdev->devHead.display->bus_type;

    /*
     ************************************************************************
     *  Having picked up the control variables, we now analyze them into
     *  cases and perform sanity checks.
     ************************************************************************
     */

    /*
     *  If srval is untouched (still -1), then we have an older Microchannel
     *  driver.  Set up the control variables accordingly.
     */
    if (srval == -1 && pmap == NULL)		/* if old timey driver */
    {
	srval = MICROCHANNEL_SEG_REG | MICROCHANNEL_STD_BUID;

	access_ctrl = 1;			/* but, we set it anyway */

	assert (resource_id == 0);

	bus_type = DISP_BUS_MCA;		/* but, we set it anyway */
    }

    /*
     *  Compute bus protection type, and perform sanity checks.
     */
    if (access_ctrl)
    {
	switch (bus_type)
	{
	    case DISP_BUS_MCA:
		if (__power_pc ())
		    busprot_type = _BUSPRT_SR_MC;
		else
		    busprot_type = _BUSPRT_CSR15;

		assert (resource_id == 0);		/* no RID's used */

		assert (pmap == NULL);

		break;

	    case DISP_BUS_PPC:
		assert (__power_pc () && dwa_device);

		if (__power_601 ())
		{
		    if (pmap != NULL)
		    {
			/*
			 *  In case a platform independent driver gives
			 *  an io_map, we must validate his info that way.
			 *  Since this is going to run on a system which
			 *  can only protect an aligned segment of 256 MBytes,
			 *  we verify this.
			 */
			assert (pmap->size == (1L << 28)        &&
				pmap->busaddr % pmap->size == 0    );

			/* arbitrary set up policy, allow writes */
			assert (!(pmap->flags & IOM_RDONLY));

			/*
			 *  If an io_map has been set up, then we compute
			 *  an entirely new srval for 601 processor bus access.
			 */
			srval = BUID_7F_SRVAL (pmap->busaddr);
		    }

		    busprot_type = _BUSPRT_7F_XID;
		}
		else
		{
		    assert (pmap != NULL);

		    /* all addresses/sizes must be properly granular */
		    assert (pmap->size >= MIN_BAT_SIZE           && /* min */
			    pmap->size <= MAX_BAT_SIZE           && /* max */
			    ((pmap->size - 1) & pmap->size) == 0 && /* pwr 2 */
			    pmap->busaddr % pmap->size == 0         );/* algn */

		    /* we don't allow read-only (arbitrary setup policy) */
		    assert (!(pmap->flags & IOM_RDONLY));

		    /*
		     *  We specify a suitable segment register value.
		     *  I'm not sure whether the actual value is significant,
		     *  but we use the value that was used in u_iomem_att ().
		     *
		     *  A BAT will be set up to map effective addresses which
		     *  use this segment register specifier as the uppermost
		     *  4 bits of the user generated effective address.
		     *
		     *  This segment register can be shared among several
		     *  devices as long as the total virtual space required
		     *  for them fits within a 256 MByte user segment.
		     *  Different ranges of effective address are mapped by
		     *  different BATS to different bus addresses.
		     *
		     *  For now, we use the following fixed value, since
		     *  it cannot conflict with any other segment register.
		     */
		    srval = INV_PB_SEGREG_PPC;

#if  0
		    /* this may be needed for Magenta on PCI */
		    /*
		     *  Pick up and validate the internal bus identifier.
		     *  (This is NOT the busid that goes in a segment reg.)
		     *  Extract index and region values.
		     */
		    bid = pmap->bid;

		    index = BID_INDEX (bid);
        	    assert (index < MAX_BID_INDEX);
        	    assert (BID_TYPE (bid_table[index].bid) == BID_TYPE (bid));
        	    assert (BID_NUM  (bid_table[index].bid) == BID_NUM  (bid));

		    region = BID_REGION (bid);
        	    assert (bid_table[index].num_regions > region);

		    /*
		     *  Calculate the real bus address.
		     */
		    braddr = (uint) bid_table[index].ioaddr[region] +
								pmap->busaddr; 

	    /* change assert involving busaddr above if this code is used */
		    /* verify that braddr is on size boundary	*/
		    size = pmap->size;

		    ASSERT((braddr & ~(size -1)) == braddr);
#endif

		    busprot_type = _BUSPRT_BAT_XID;
		}

		break;

	    default:
		assert (0);
	}
    }
    else
    {
	assert (!dwa_device);

	busprot_type = _BUSPRT_NO_ACCESS;
    }

    /*
     ************************************************************************
     *  Massage segment register value per platform.
     *
     *  If this is PowerPC, then clear all K bits.  The bos325 kernel gangs
     *  the 2 K bits together for this architecture in bos release 3.2.5.
     *
     *  If this is Power/Power2 architecture accessing the IOCC, then the K
     *  bit is always turned on.  If the adapter takes the whole 256MByte
     *  segment and also needs no address based access control, then the
     *  Bypass bit may be turned on.  Currently, we make this decision based
     *  on segment number:  Bypass is turned on if the segment is nonzero.
     *  (Future developments could make this decision based on a bit set in
     *  the display->flags word.)
     *
     *  Power/Power2 access to non IOCC bus controllers is undefined in this
     *  code.  The "special graphics slot" adapters exist (SGA/WGA), but the
     *  value of the K bit in the segment register for these adapters may be
     *  immaterial.  At present, the K bit is set on, just as for all other
     *  Power/Power2 graphics adapters.
     ************************************************************************
     */
    if (__power_pc ())
    {
	srval |=   PWR_TBIT_MASK;
	srval &= ~(PPC_KBIT_MASK | PWR_IBIT_MASK | PPC_RESV_MASK);
    }
    else
    {
	srval |=  (PWR_TBIT_MASK | PWR_KBIT_MASK | PWR_ADIC_MASK);
	srval &=
	    ~(PWR_IBIT_MASK | PWR_MBIT_MASK | PWR_BBIT_MASK | PWR_RESV_MASK);

	if (srval & PWR_ADDR_MASK)		/* nonzero segs assume Bypass */
	    srval |= PWR_BBIT_MASK;
    }

    /*
     ************************************************************************
     *  Synchronize with segment register values database.  This allows
     *  sharing of segment registers under certain circumstances.
     *
     *  The database is attached to the common process structure, in order
     *  to allow potential sharing for multi-head operation.
     *
     *  Sharing is based on the "pure" value of the srval at this point in
     *  the code.  This is the operational value that will be in the segment
     *  register when access to the adapter is enabled.  Exception:  For IOCC
     *  (Microchannel) adapters, the bus memory protection key field will be
     *  merged with the value here for operational use in the segment register.
     *  This is done by the kernel in response to the rcm's bus protection
     *  registration packet (see below).  Also, when a graphics fault is being
     *  forced, the kernel may load other strange values in the registers.
     *  In fact, we may do so below, depending on platform type.  These values
     *  have nothing to do with the sharing capability.
     ************************************************************************
     */

    /* search sr database for value */
    segreg = pcproc->pSegreg;
    while (segreg != NULL)
    {
	if (segreg->srval == srval)
		break;

	segreg = segreg->next;
    }

    /* if value not found, allocate new segment register for process */
    if (segreg == NULL)
    {
	segreg = xmalloc (sizeof (struct segreg), 3, pinned_heap);
	if (segreg == NULL)
	{
	    rcm_kill_proc (pproc);
	    return (ENOMEM);
	}

	/* fill in the record */
	segreg->srval   = srval;
	segreg->usage   = 1;

	/*
	 *  Decide what to fill the segment register with for initial value.
	 *  For certain switching adapters on certain platforms, this value
	 *  is different from the operational value which will be put into
	 *  the segment register (from the dispauth registration packet) once
	 *  bus access is granted.
	 */
	switch (busprot_type)	/* (platform x iotype) */
	{
	    /* define values for as_att */

	    case _BUSPRT_CSR15:
		srval_as_att = srval;

		break;

	    case _BUSPRT_SR_MC:
		if (dwa_device)
		    srval_as_att = INV_MC_SEGREG;
		else
		    srval_as_att = srval;

		break;

	    case _BUSPRT_7F_XID:
		srval_as_att = INV_PB_SEGREG_PPC;

		break;

	    case _BUSPRT_BAT_XID:
		srval_as_att = INV_PB_SEGREG_PPC;

		break;

	    case _BUSPRT_NO_ACCESS:
		srval_as_att = srval;

		break;

	    default:
		assert (0);
	}

	/*
	 *  Now load the seg reg.
	 *
	 *  Note that as_att may return NULL if no more segment registers
	 *  are available.
	 */
	segreg->segment = as_att (getadsp (), srval_as_att, (caddr_t) 0);

	/* link into chain  (rcm_kill_proc will pick it up) */
	segreg->next    = pcproc->pSegreg;
	pcproc->pSegreg = segreg;
    }
    else
	segreg->usage++;		/* reuse by updating usage count */

    /*
     *  Save the effective address specifying the segment register in
     *  the rcmprocess structure, and pass it back to the caller.
     *
     *  Setting pSegreg non-NULL also tells the rcm_kill_proc error
     *  handler to release the assignments.
     */
    pproc->procHead.pSegreg = segreg;

    if (segreg->segment == NULL || suword (&parg->segment, segreg->segment))
    {
	rcm_kill_proc (pproc);
	BUGPR(("###### make_gp ERROR returning segment info\n"));
	return (EFAULT);
    }

    /* if not DWA device, or device reserved */
    /*
     *  Specifying DEV_RESERVED for certain switching adapters may not work.
     *  The setup for switching and hi/lo water mark is always required for
     *  The PED/LEGA family of adapters.
     */
    if (!pdev->devHead.display->dwa_device
	|| (pdev->devHead.flags & DEV_RESERVED)) {

	ioRange     range;
	struct  rcmbusprot  *rcmbusprot;

	BUGLPR(dbg_rcmgp,BUGACT,("====== not a DWA device, or superuser\n"));

	/* turn on access to every domain so that no fault occurs */

	/*
	 *  Supported "nondwa" adapter list:
	 *
	 *	Skyway		(RS1 Microchannel w/ Bus I/O capability)
	 *	SGA/WGA		(RSC (=RS1) w/ "special graphics slot")
	 *	Baby Blue	(601 and MCA)
	 *	Neptune		(MCA)
	 *	Perhaps certain other adapters in diagnostic mode(?).
	 */

	/*
	 *  Set up bus access packets for those adapters which require kernel
	 *  assist, so that access is always granted and there will be no
	 *  graphics faults.
	 * 
	 *  This is applicable to Microchannel adapters.  However, note that the
	 *  RCM will not always set up properly for all switching adapters when
	 *  device reservation is called for.  (The PED family cannot be set
	 *  up in non-switching mode because of the highwatermark processing).
	 */
	if (access_ctrl)
	for (i=0; i < pdev->devHead.num_domains; i++)
	{
	    /*
	     *  Set up bus protection packet for each domain.
	     */
	    rcmbusprot = xmalloc (sizeof (struct rcmbusprot), 3, pinned_heap);
	    if (rcmbusprot == NULL)
	    {
	        rcm_kill_proc (pproc);
	        return (ENOMEM);
	    }

	    bzero (rcmbusprot, sizeof (struct rcmbusprot));

	    switch (busprot_type)
	    {
		case  _BUSPRT_CSR15:
	            BUSPRT_TYPE   (&rcmbusprot->busProt) = _BUSPRT_CSR15;
	            BUSPRT_EADDR  (&rcmbusprot->busProt) = (unsigned long)
					pproc->procHead.pSegreg->segment;
	            BUSPRT_IOCC_SR(&rcmbusprot->busProt) = CVT_IOCC (srval);
	            BUSPRT_CSR15  (&rcmbusprot->busProt) = pdev->domain[i].auth;
	            BUSPRT_LIMIT  (&rcmbusprot->busProt) =
						pdev->domain[i].range.range;
		    break;

		case  _BUSPRT_SR_MC:
	            BUSPRT_TYPE    (&rcmbusprot->busProt) = _BUSPRT_SR_MC;
	            BUSPRT_EADDR   (&rcmbusprot->busProt) = (unsigned long)
					pproc->procHead.pSegreg->segment;
	            BUSPRT_MC_SR   (&rcmbusprot->busProt) =
						srval | pdev->domain[i].auth;

		    /* PowerPC H/W does not provide for limits checking */
		    if (pdev->domain[i].range.range != IOCC_NO_IO)
		    {
		        xmfree ((caddr_t) rcmbusprot, pinned_heap);
		        rcm_kill_proc (pproc);
		        return (EINVAL);
		    }

		    break;

		default:
		    assert (0);
	    }

	    /* register the I/O bus protection packet */
	    if (reg_display_acc (&rcmbusprot->busProt) == -1)
	    {
		xmfree (rcmbusprot, pinned_heap);
		rcm_kill_proc (pproc);
		return (ENOMEM);
	    }

	    /* grant immediate and continuous access */
	    grant_display_owner (&rcmbusprot->busProt);

	    /* mark the domain */
	    rcmbusprot->domain = i;

	    /*
	     *  Link new rcmbusprot structure to rcmprocess structure.
	     *
	     *  This also tells rcm_kill_proc of its existence.
	     */
	    rcmbusprot->next = pproc->procHead.pRcmbusprot;
	    pproc->procHead.pRcmbusprot = rcmbusprot;
	}

	/* set the gp as the current for all domains, with no rcx,
	   so that interrupts work correctly */
		/* is it enough to have just a process and no rcx ?? */
	for (i=0; i < pdev->devHead.num_domains; i++) {
		pdev->domain[i].pCurProc = pproc;
		pdev->domain[i].pCur = NULL;
	}

	/* make sure we have a valid pproc pointer for DMA, events */
	pdev->devHead.display->cur_rcm = pproc;

    } else {  /* a DWA device */

	BUGLPR(dbg_rcmgp,BUGACT,("====== a DWA device\n"));

	/*
	 *  Remember that rcm_kill_proc (error cleanup) will
	 *  free the pproc and all prcx's linked to it, as
	 *  well as call the DD level entry for all of them.
	 */

	/*
	 *  The supported switching adapter list:
	 *
	 *	Sabine		(Microchannel)
	 *	GTO		(Microchannel)
	 *	PED/LEGA	(Microchannel)
	 *	Ruby		(Microchannel)
	 *	Magenta		(60X)
	 *
	 *  Set up bus protection packet and null context for each domain.
	 */

	assert (access_ctrl);

	for (i=0; i < pdev->devHead.num_domains; i++) {

	    rcxPtr prcx;
	    struct  rcmbusprot  *rcmbusprot;

	    /*
	     *  Set up bus protection packet for each domain.
	     */
	    rcmbusprot = xmalloc (sizeof (struct rcmbusprot), 4, pinned_heap);
	    if (rcmbusprot == NULL)
	    {
		rcm_kill_proc (pproc);
		return (ENOMEM);
	    }

	    bzero (rcmbusprot, sizeof (struct rcmbusprot));

	    /* No limits checking switching adapters allowed.  Not possible */
	    if (pdev->domain[i].range.range != IOCC_NO_IO)
	    {
		xmfree ((caddr_t) rcmbusprot, pinned_heap);
		rcm_kill_proc (pproc);
		return (EINVAL);
	    }

	    switch (busprot_type)
	    {
		case  _BUSPRT_CSR15:
		    BUSPRT_TYPE   (&rcmbusprot->busProt) = _BUSPRT_CSR15;
		    BUSPRT_EADDR  (&rcmbusprot->busProt) = (unsigned long)
					pproc->procHead.pSegreg->segment;
		    BUSPRT_IOCC_SR(&rcmbusprot->busProt) = CVT_IOCC (srval);
		    BUSPRT_CSR15  (&rcmbusprot->busProt) = pdev->domain[i].auth;
		    BUSPRT_LIMIT  (&rcmbusprot->busProt) = IOCC_NO_IO;

		    pdev->devHead.fault_type = EXCEPT_IO;

		    break;

		case  _BUSPRT_SR_MC:
		    BUSPRT_TYPE  (&rcmbusprot->busProt) = _BUSPRT_SR_MC;
		    BUSPRT_EADDR (&rcmbusprot->busProt) = (unsigned long)
					pproc->procHead.pSegreg->segment;
		    BUSPRT_MC_SR (&rcmbusprot->busProt) = srval |
						pdev->domain[i].auth;

		    pdev->devHead.fault_type = EXCEPT_IO;

		    break;

		case  _BUSPRT_7F_XID:
		    BUSPRT_TYPE    (&rcmbusprot->busProt)   = _BUSPRT_7F_XID;
		    BUSPRT_EADDR   (&rcmbusprot->busProt)   = (unsigned long)
					pproc->procHead.pSegreg->segment;
		    BUSPRT_7F_XID_SR(&rcmbusprot->busProt)  = srval;
		    BUSPRT_7F_XID_EAR(&rcmbusprot->busProt) =
						resource_id | EAR_ENABLE_MASK;

		    pdev->devHead.fault_type = EXCEPT_GRAPHICS_SID;

		    break;

		case  _BUSPRT_BAT_XID:

#define WIMG_IMG	0x7	/* needs def'n in sys/vmker.h */

		    BUSPRT_TYPE    (&rcmbusprot->busProt)     = _BUSPRT_BAT_XID;
		    BUSPRT_EADDR   (&rcmbusprot->busProt)     = (unsigned long)
					pproc->procHead.pSegreg->segment;
		    BUSPRT_BAT_XID_BATU(&rcmbusprot->busProt) =
			DBATU ((unsigned int) pproc->procHead.pSegreg->segment,
				(((pmap->size/MIN_BAT_SIZE)-1)<<2), BT_VP);
		    BUSPRT_BAT_XID_BATL(&rcmbusprot->busProt) =
				DBATL (pmap->busaddr, WIMG_IMG, BT_WRITE);
		    BUSPRT_BAT_XID_EAR(&rcmbusprot->busProt)  =
						resource_id | EAR_ENABLE_MASK;

		    pdev->devHead.fault_type = EXCEPT_GRAPHICS_SID;

		    break;

		default:
		    assert (0);
	    }

	    /* register the I/O bus protection packet */
	    if (reg_display_acc (&rcmbusprot->busProt) == -1)
	    {
		xmfree ((caddr_t) rcmbusprot, pinned_heap);
		rcm_kill_proc (pproc);
		return (ENOMEM);
	    }

	    /* mark the domain */
	    rcmbusprot->domain = i;

	    /*
	     *  Link new rcmbusprot structure to rcmprocess structure.
	     *
	     *  This also tells rcm_kill_proc of its existence.
	     */
	    rcmbusprot->next = pproc->procHead.pRcmbusprot;
	    pproc->procHead.pRcmbusprot = rcmbusprot;

	    /*
	     *  Now create the NULL context for the domain.
	     */
	    prcx = xmalloc (sizeof(struct _rcx), 3, pinned_heap);
	    if (prcx == NULL) {
		/* detach segment and free proc and other created
		   rcx structures */
		BUGPR(("###### make_gp ERROR xmalloc rcx %d\n", i));
		rcm_kill_proc (pproc);
		return (ENOMEM);
	    }

	    /* exit must free prcx until it's linked for rcm_kill_proc */

	    prcx->pNextFlt = NULL;
	    prcx->pProc = pproc;
	    prcx->priority = pproc->procHead.priority;
	    prcx->pData = NULL;
	    prcx->domain = i;
	    prcx->pDomain = &(pdev->domain[i]);
	    prcx->pWG = NULL;
	    prcx->pWA = NULL;
	    prcx->pLinkWG = NULL;
	    prcx->pLinkWA = NULL;
	    prcx->pRcxph = NULL;
	    prcx->flags = RCX_NULL;
	    prcx->timeslice = RCM_DEFAULT_TIMESLICE;
	    prcx->pDomainLock = NULL;

	    /* make the null rcx current for the gp on the domain */
	    pproc->pDomainCur[i] = prcx;

	    /* call the device dependent code to let it create the
	       private area for the rcx */
	    /************** WARNING ***********************************/
	    /* it is assumed that device specific create functions do */
	    /* not have to be guarded, because they will not cause a  */
	    /* context switch					      */
	    /************** WARNING ***********************************/
	    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
	    pa->error = (pdev->devHead.display->create_rcx)
						   (pdev, prcx, pa, NULL);
	    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
	    if (pa->error) {
		suword (&parg->error, pa->error);
		BUGPR(("###### create_rcx ERROR dd create_rcx error, %d \n",
		       pa->error));
		xmfree ((caddr_t) prcx, pinned_heap);	/* not yet linked */
		rcm_kill_proc (pproc);
		return (EIO);
	    }

	    prcx->pNext = pproc->procHead.pRcx;
	    pproc->procHead.pRcx = prcx;

	    /* now rcm_kill_proc will pick it up and call DD for it */
	}
    }

    return (0);
}

/* ============================================================= */
/* FUNCTION: gsc_unmake_gp
*/
/* PURPOSE: unmakes a process a graphics process
*/
/* DESCRIPTION:
*/
/* INVOCATION:
    int gsc_unmake_gp (pd, parg, parm1)
	struct phys_displays    *pd;
	unmake_gp		*parg;
	int			parm1;
*/
/* CALLS:
*/
/* DATA:
	virtual terminal structure and RCM/DDH structure
*/
/* RETURNS:
	0 if successful
	ERRNO if unsuccessful
*/
/* OUTPUT:
	error code
*/
/* RELATED INFORMATION:
*/
/* NOTES:
	this function and the exit code are probably similar if not
	identical
*/

int gsc_unmake_gp (pd, parg, parm1)

    struct phys_displays *pd;
    unmake_gp		*parg;
    int			parm1;
{
    unmake_gp	    a;
    gscDevPtr	    pdev;
    int		    *prev_funl = (int *) parm1;

    /* get argument */
    if (copyin (parg, &a, sizeof (a))) {
	BUGPR(("###### unmake_gp ERROR copyin arg \n"));
	return (EFAULT);
    }

    /* set the device pointer for further operations */
    SET_PDEV(pd,pdev);

    return (rcm_unmake_gp(pdev, prev_funl));
}


/* used in next function */
static void watchwake (), term_clients ();
static int watchwait ();

/*
 *  Timeout for unmake-gp of leader to see all clients unmake first (currently
 *  30 seconds).
 *
 *  If any clients haven't performed an unmake-gp after the initial timeout
 *  expires, any such remaining clients will be killed with SIGKILL.  The
 *  RCM will wait 10 seconds to allow all clients to finish the unmake-gp.
 */
#define  RCM_SYNC_WATCH_TERM  30	/* wait for SIGTERM to take effect */
#define  RCM_SYNC_WATCH_KILL  10	/* wait for SIGKILL to take effect */

int rcm_unmake_gp(pdev, prev_funl)
gscDevPtr	pdev;
int		*prev_funl;
{
    rcmProcPtr	    pproc, pprocd;
    gscComProcPtr   pcproc;
    int 	    ret, error, domain, lock_stat, old_int;
    pid_t	    cur_pid;
    rcmWAPtr	    pwa;
    rcmWGPtr	    pwg, pwg_n;
    struct _partList *part;
    ulong	    flags;
    struct phys_displays *pd;

    rcm_wg_hash_t   *wg_hash, *table_end ;


    BUGLPR(dbg_rcmgp,BUGNFO,("\n==== Enter unmake_gp\n"));
    cur_pid = getpid();
    gsctrace (UNMAKE_GP, PTID_ENTRY);

    /* this must be a graphics process */
    FIND_GP (pdev, pprocd);
    if(!pprocd)
	    return EINVAL;		/* not a gp */

    /* First thing to do is to mark this process as being in unmake_gp */
    /* disable FIND_GP for this rcm process */
    pprocd->procHead.flags |= PROC_UNMAKE_GP;

    /*
     *  Release any per-process DMA resources.  Ignore any d_complete error
     *  that might return from a master device.
     */
    (void) dma_free (pdev, pprocd);

    BUGLPR(dbg_rcmgp,BUGACT,
	   ("====== unmake_gp found proc, pprocd=0x%x\n", pprocd));

    RCM_TRACE(0x910,getpid(),pprocd,0);

    /*
     *  Delete all contexts for this graphics process on this device.
     *
     *  This has to be done while any locks ARE STILL IN PLACE!  This
     *  currently amounts to being able to handle these conditions:
     *
     *  1)  Fast domain lock set, or
     *  2)  Adapter locked, or
     *  3)  Std domain locked.  This is currently not used and is NOT
     *      ANALYZED for the following code.
     *
     *  Maintenance note: The following has certain structural similarities
     *  to rcm_kill_proc, but is more complex.
     */
    while (pprocd->procHead.pRcx != NULL)
    {
	ret = rcm_delete_rcx (pdev, pprocd->procHead.pRcx, &error);
	/* note that rcm_delete_rcx will fix the linked list so that only
	   need to look at the head */
    }

    /*  Release device and domain locks. */

    /*
     *  Current logic in the guard/unguard_dom functions implies that the
     *  domain may be guarded by a process, and in addition, be either
     *  locked or dev_locked by that same process.  (Currently, the domain
     *  locking facility is not used and is scheduled for removal.  The
     *  "fast domain locking" capability used for gemini is implemented
     *  in another way).
     */

    /*
     *  Remove device lock, if locked by this process.  This call, if
     *  applicable, will unguard the domains on the device and erase all
     *  memory of locking by the current process.  This means that all
     *  guards of any kind are released also, if domain device is released.
     */
    (void) rcm_unlock_pdev (pdev, pprocd, PDEV_UNNEST | PDEV_COND);

    /*
     *  Remove regular lock (not just guard), if any.  This will not have
     *  any effect if the device was dev-locked.  See above paragraph.
     *  If this code unlocks a domain, any guard for it is also removed.
     *  (The domain locking via this mechanism is scheduled for removal).
     *  Since there is no removal of plain domain guards, they are not
     *  presumed to be a problem.  Plain guards are not supposed to be
     *  left on by a process, when it returns to user mode.  We can
     *  only get here from user mode by direct system call or state change.)
     */
    for (domain = 0; domain < pdev->devHead.num_domains; domain++)
    {
	if (pdev->domain[domain].pLockProc == pprocd)
	    unguard_dom (&pdev->domain[domain], UNGUARD_UNLOCK);
    }

    /*
     *  Begin to release entities "owned" by this rcm process structure.
     *  Try to take down the categories of entities in inverse order to the
     *  order of establishment.  That is, take down pprocd last.  The most
     *  volatile entities (contexts) were taken down above.
     */

    /*
     *  'term_clients ()' will send a SIGTERM to all the clients on this head
     *  (other than ourselves), and will wait until the other clients complete
     *  their 'unmake-gp' operations.
     *
     *  Except for context deletion and lock release (done above), this makes
     *  the leader go down (making most of his DD calls) last, even though he
     *  might have started down first.
     *
     *  Lock deletion has to occur in X before this delay, lest X block one
     *  of the clients going down.
     *
     *  This code depends on the processes receiving the signals not be
     *  blocked (for long).  No attempt is made here to releave any blockage
     *  due to uexblock, since that is the responsibility of the device driver,
     *  and/or fault handler to not let a process hang.
     *
     *  Actually, most X clients receive some kind of signal (or socket
     *  closure?) that causes them to go down.  However, it is not known
     *  to me that there is any defined protocol for this.  The net result
     *  is that most clients already know by this point that the process
     *  leader is going down, and most of the clients may already be down
     *  by now.  However, since there is no certain known protocol to en-
     *  force this, we send a SIGTERM to all clients that are still up at
     *  this point.  If that doesn't do the job, then they get a SIGKILL!
     */
    if (pprocd->procHead.flags & PROC_GP_LEADER)
    {
	term_clients (pdev, cur_pid,
			RCM_SYNC_WATCH_TERM, RCM_SYNC_WATCH_KILL);
    }

    /*
     *  Maintenance note:
     *
     *  The following is analogous to rcm_kill_proc, but more complex.
     */

    /*
     *  Even though the following loop is protected by pdev lock, any
     *  of the hash table linked lists can be updated instantaneously
     *  by 'create-win-geom' for another process.  The pdev lock was
     *  removed for optimization purposes in 'create-win-geom'.  The
     *  fact means that the calculation of pwg_n in the loop below can
     *  sometimes skip a new member of the list.  This is not a logic
     *  problem at this time, though it may be a maintenance problem in
     *  the long run.
     */

    /* rcm lock device to prevent wg list from corrosion */
    rcm_lock_pdev (pdev, pprocd, 0);

    /* delete all wg created by the gp */
    table_end = &(pdev->devHead.wg_hash_table->entry[RCM_WG_HASH_SIZE]) ;
    for (wg_hash = &(pdev->devHead.wg_hash_table->entry[0]) ;
         wg_hash < table_end;
         wg_hash++)
    {
        for (   pwg = (rcmWGPtr) wg_hash->pWG ;
                pwg != NULL;
                pwg = pwg_n)
        {
        pwg_n = pwg->pNext ; 		/* Next is destroyed by delete (free)*/

        if (pwg->pProc != pprocd)       /* skip the ones not ours */
                continue;

        /* if any rendering context is attached to this geom, just set flag */
        if (pwg->pHead)
            pwg->flags |= WG_DELETED;
        else
            ret = rcm_delete_win_geom (pdev, pwg, &error);
        }
    }

    /* rcm unlock device */
    rcm_unlock_pdev (pdev, pprocd, 0);

    /* delete all wa created by the gp */
    while (pprocd->procHead.pWA != NULL) {
	ret = rcm_delete_win_attr (pdev, pprocd, pprocd->procHead.pWA, &error);
	/* note that rcm_delete_win_attr will fix the linked list so
	   that only need to look at the head */
    }

    /* call the device dependent unmake_gp function */
    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
    if (pdev->devHead.display->unmake_gp)
	    (pdev->devHead.display->unmake_gp) (pdev, pprocd);
    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);

    /*  End of rcm_kill_proc similitude, except for xmfree of pprocd */

    /*
     *  Release event resources.
     *
     *  This depends on the above DD level unmake_gp stopping
     *  any interrupt/event driven work that might be pending.
     */
    (void) event_free (pdev, pprocd);

    /*
     *  If the leader is going away, then the pointer to the common
     *  process user buffer header array should be NULL'd out.
     */
    if (pprocd->procHead.flags & PROC_GP_LEADER)
	pdev->devHead.pusrbufhdr = NULL;

    /*
     *  Pprocd is now free of attachments.
     *  Remove it from the pdev list.
     */
    rcm_lock_pdev (pdev, pprocd, 0);

    old_int = i_disable (INTMAX);

    pproc = pdev->devHead.pProc;	/* first rcm proc struct */
    assert (pproc);

    if (pproc->procHead.pid == cur_pid)	/* first in list */
    {
	pprocd = pproc;
	pdev->devHead.pProc = pproc->procHead.pNext;
    }
    else				/* not first in list, search the rest */
    {
	/* is "next" one a match? */
	for (; pproc->procHead.pNext != NULL; pproc = pproc->procHead.pNext)
	    if (pproc->procHead.pNext->procHead.pid == cur_pid)
		break;

	assert (pproc->procHead.pNext);

	/* unlink gp */
	pprocd = pproc->procHead.pNext;
	pproc->procHead.pNext = pprocd->procHead.pNext;
    }

    /* end of rcm process structure update (critical section) */
    i_enable (old_int);

    rcm_unlock_pdev (pdev, pprocd, 0);

    /*
     *  Delete all relevant context parts (under the common lock).
     */

    /* rcm lock common to prevent lists from corrosion */
    RCM_LOCK (&comAnc.rlock, lock_stat);

    /* delete all context parts created by this process */
    part = pprocd->procHead.pParts;
    while (part)
    {
	struct _partList *part_next;

	BUGLPR(dbg_rcmgp,BUGNFO,
	   ("unmake_gp: delete part 0x%x part->glob_id 0x%x pprocd 0x%x\n",
					part, part->pRcxp->glob_id, pprocd));

	part_next = part->pNext;

	/* this function call will delete what part points to */
	(void) rcm_delete_rcxp (pdev, part, NULL, &error, pprocd);

	part = part_next;
    }

    /* rcm unlock com */
    RCM_UNLOCK (&comAnc.rlock, lock_stat);

    /*
     *  If not leader, send wakeup to possibly sleeping leader.
     */
    if (!(pprocd->procHead.flags & PROC_GP_LEADER))
	e_wakeup (&pdev->devHead.sync_sleep);

    /* we need this a couple of times after freeing pprocd */
    pcproc = pprocd->procHead.pCproc;

    /* deregister the bus protection packets */
    detach_busprot (pprocd);

    /* release usage of segreg database due to pprocd */
    detach_seg_reg (pprocd);

    /* free the gp structure */
    xmfree ((caddr_t) pprocd, pinned_heap);

    /* rcm lock common to prevent lists from corrosion */
    RCM_LOCK (&comAnc.rlock, lock_stat);

    /*
     *  Possibly do away with device structure.
     *  Save flags for later trace point
     */
    flags = pdev->devHead.flags;
    pd = pdev->devHead.display;
    dev_term (pd, pcproc);

    /*
     *  Possibly do away with common process structure.
     */
    cproc_term (pcproc, prev_funl);

    /*
     *  Possibly do away with common structure.
     */
    com_term (pd);

    /* rcm unlock com */
    RCM_UNLOCK (&comAnc.rlock, lock_stat);

    RCM_TRACE(0x911,getpid(),flags,0);

    BUGLPR(dbg_rcmgp,BUGNFO,
	   ("==== Exit unmake_gp... \n\n"));

    gsctrace (UNMAKE_GP, PTID_EXIT);

    /* return */
    return (0);
}


/* used in watchwait and watchdog */
struct sync_watch
{
    struct watchdog  wd;
    gscDevPtr        pdev;
};


static void  term_clients (pdev, cur_pid, timeout_term, timeout_kill)
gscDev  *pdev;
pid_t    cur_pid;
int	 timeout_term, timeout_kill;
{
	rcmProc  *pproc;
	int  old_int;
	struct sync_watch *psync_watch;

	psync_watch = xmalloc (sizeof (struct sync_watch), 3, pinned_heap);

	/*  atomic operations */
	old_int = i_disable (INTMAX);

        /*
         *  Set the DEV_SIGNAL_SENT flag and signal all processes that are
         *  on the rcm process list of the device.  All processes that attempt
	 *  to become graphics processes after the signal has gone out will see
	 *  the flag set and will unmake themselves and return an error.  This
	 *  handles race conditions where a process is attempting to become
	 *  graphical while the graphical subsystem is coming down.  Such a
	 *  process doesn't see this SIGTERM.
         */
	pdev->devHead.flags |= DEV_SIGNAL_SENT;

	/*
	 *  Signal the client graphical processes ON THIS HEAD that haven't
	 *  been signaled already.
         *
         *  We only signal any process once.  To avoid causing trouble with
	 *  multihead processes, we set a bit in the common process structure
	 *  to inhibit future signals by this mechanism.  This means that a
	 *  multihead process will only get one signal.  NOTE:  Since the only
	 *  multihead process currently supported is the graphics process
	 *  leader (which doesn't get signalled) this special check does
	 *  nothing at this time.
         *
         *  INTERESTING NOTE:  Just because a process gets the SIGTERM doesn't
         *  mean it will terminate!  If it doesn't, we SIGKILL it below!
	 */
	for (pproc = pdev->devHead.pProc;
	     pproc != NULL;
	     pproc = pproc->procHead.pNext)
	{
	    if (pproc->procHead.pid != cur_pid                       &&
		!(pproc->procHead.pCproc->flags & COMPROC_SIGNALLED)    )
	    {
		pidsig(pproc->procHead.pid, SIGTERM);
		pproc->procHead.pCproc->flags |= COMPROC_SIGNALLED;
	    }
	}

        /*
         *  Sleep, waiting for all other graphical processes on this device
	 *  (virtual terminal) to unmake themselves.
	 *
	 *  The sleep is watchdogged, but is not sensitive to signals, since
	 *  any inadvertent touching of the keyboard, mouse, or other gio
	 *  device will generate a signal.  We do not want the sleep broken
	 *  by such a signal.
	 *
	 *  This watchdog value is kept significantly less than 30 seconds,
	 *  because the latter value is thought to be compatible with the
	 *  maximum patience of watchdogs in the HFT for retract and revoke
	 *  operations.
	 *
	 *  If the timeout takes place, then all remaining graphics processes
	 *  (except the leader) will be SIGKILLed.  This is necessary because
	 *  bad things can happen if a graphics process is rendering when
	 *  the terminal reverts to KSR mode.
         */
	if (watchwait (pdev, timeout_term, cur_pid, psync_watch) == EBUSY)
	{
	    for (pproc = pdev->devHead.pProc;
	         pproc != NULL;
	         pproc = pproc->procHead.pNext)
	    {
	        if (pproc->procHead.pid != cur_pid)
		    pidsig(pproc->procHead.pid, SIGKILL);
	    }

	    watchwait (pdev, timeout_kill, cur_pid, psync_watch);
	}

	i_enable (old_int);

	if (psync_watch != NULL)
	    xmfree ((caddr_t) psync_watch, pinned_heap);
}


/*
 *  watchwait - delay
 */
static int watchwait (pdev, timeout, cur_pid, psync_watch)
gscDevPtr  pdev;
int        timeout;
pid_t	   cur_pid;
struct sync_watch *psync_watch;
{
    rcmProc  *pproc;
    int  rc = 0;

    if (psync_watch != NULL)
    {
	/* initialize and insert the watchdog into the system */
	psync_watch->wd.next = psync_watch->wd.prev = NULL;
	psync_watch->wd.count = 0;
	psync_watch->wd.restart = timeout;
	psync_watch->wd.func = watchwake;
	psync_watch->pdev = pdev;

	w_init  (&psync_watch->wd);
	w_start (&psync_watch->wd);

	/* the leader is still on the list */
	pproc = pdev->devHead.pProc;
	while (pproc)
	{
	    /*  skip over the leader, which may be on the list */
	    if (pproc->procHead.pid == cur_pid)
	    {
		pproc = pproc->procHead.pNext;

		continue;
	    }

	    /*  someone who is not the leader is on the rcm process list */

	    /*  timed out?  */
	    if (psync_watch->wd.count <= 0)
	    {
		rc = EBUSY;
		break;
	    }

	    /*  waked by client or timeout */
	    e_sleep (&pdev->devHead.sync_sleep, EVENT_SHORT);

	    /*  start at the top of the list again */
	    pproc = pdev->devHead.pProc;
	}

	/* clear the watchdog from the system */
	w_stop  (&psync_watch->wd);
	w_clear (&psync_watch->wd);
    }
    else
	rc = ENOMEM;

    return  rc;
}


/*
 *  watchwake - watchdog wakeup for unmaking the leader.
 */
static void watchwake (struct watchdog *pwd)
{
    struct sync_watch *psync_watch = (struct sync_watch *) pwd;

    e_wakeup (&psync_watch->pdev->devHead.sync_sleep);
}


static detach_seg_reg (rcmProcPtr pproc)
{
    struct segreg  *segreg, *segreg_pre;

#ifdef  U_IOMEM_ATT
    if (pproc->procHead.pGSC->devHead.display->io_map != NULL  &&
	pproc->procHead.pGSC->devHead.display->dwa_device == 0    )
    {
	u_iomem_det (pproc->procHead.pSegreg);

	return;
    }
#endif

    /* decrement the usage count on our seg reg packet */
    pproc->procHead.pSegreg->usage--;
    if (pproc->procHead.pSegreg->usage <= 0)		/* if no more users */
    {
        segreg = pproc->procHead.pCproc->pSegreg;	/* top of list */
        segreg_pre = NULL;
        while (segreg != NULL)				/* look up segreg_pre */
        {
	    if (segreg == pproc->procHead.pSegreg)
	        break;

	    segreg_pre = segreg;
	    segreg = segreg->next;
        }

        if (segreg == NULL)
	    assert (0);

	if (segreg_pre == NULL)
	    pproc->procHead.pCproc->pSegreg = segreg->next;
	else
	    segreg_pre->next = segreg->next;

	/* as_att may have run out of registers */
	if (segreg->segment != NULL)
	    as_det (getadsp (), segreg->segment);

	xmfree ((caddr_t) segreg, pinned_heap);
    }
}

static detach_busprot (rcmProcPtr pproc)
{
    struct rcmbusprot  *rcmbusprot, *rcmbusprot_n;

    rcmbusprot = pproc->procHead.pRcmbusprot;
    pproc->procHead.pRcmbusprot = NULL;

    while (rcmbusprot != NULL)
    {
	rcmbusprot_n = rcmbusprot->next;

	unreg_display_acc (&rcmbusprot->busProt);

	xmfree ((caddr_t) rcmbusprot, pinned_heap);

	rcmbusprot = rcmbusprot_n;
    }
}

/* ============================================================= */
/* FUNCTION: rcm_state_change
*/
/* PURPOSE: handles changes in the state of any process that can affect
            graphics.
*/
/* DESCRIPTION:
	FOR TERMINATE:
	    It cleans up graphics state after any process that can
	    affect graphics exits.

	    Calls other functions which do the work:  gp_state_change
	    to perform any needed unmake-gp operations, and devno_state_change
	    to release device numbers.

	FOR OTHERS:
	    At this time does nothing.
*/
/* INVOCATION:
    void rcm_state_change (handler, type, id)
	struct proch	*handler;
	int		type;
	pid_t or t_id	id;
*/
/* CALLS:
*/
/* DATA:
	virtual terminal structure and RCM/DDH structure
*/
/* RETURNS:
	NONE
*/
/* OUTPUT:
*/
/* RELATED INFORMATION:
*/
/* NOTES:
	This function must be registered to catch the state changes
	for all processes.  Registration is performed when the rcm
	device driver is configured.

	All state change functions are kept here so that they will be
	in pinned code.
*/

void gp_state_change (), devno_state_change ();

union exec_id
{
    pid_t pid;
    tid_t tid;
};

void rcm_state_change (handler, type, id)
struct proch    *handler;
int		type;
union exec_id   id;
{
    /*
     *  We should key on the thread termination so that thread abort of
     *  a graphical thread cleans up graphical state immediately, without
     *  waiting till the whole process terminates.  This allows the process
     *  to start another graphical thread.  From a graphics standpoint, it
     *  is as if the whole process went down: not only will the unmake-gp
     *  be done, but the head reservation will be released also.
     *
     *  We are running on the thread being terminated.
     *
     *  Since we can enter here for any thread, in particular for nongraphical
     *  threads of a graphical process, we must be sure and check the tid,
     *  not just the pid.
     */
    if (type == THREAD_TERMINATE)
    {
	BUGLPR(dbg_rcmgp,BUGNFO,
	   ("rcm_state_change: tid %d term'd\n", id.tid));

	/*
	 *  If we are SMP and aren't running on the graphics processor, then
	 *  there is nothing to do.
	 *
	 *  NOTE:  We don't know whether this is an (ex)-graphical process at
	 *  all without information output from gp_state_change.  If the
	 *  local 'prev_funl' flag gets cleared, then we unfunnel.  This
	 *  can only happen on an MP, since UNI's are "always" funnelled.
	 */
	if (!__power_mp () || am_i_funnelled ())
	{
	    int prev_funl = 1;			/* uniprocessor default */

	    gp_state_change (id.tid, &prev_funl);
	    devno_state_change (id.tid);

	    /*
	     *  Since funnelling status is initialized to TRUE for
	     *  for uniprocessors, if prev_funl has been reset then
	     *  we are on SMP.
	     *
	     *  Perhaps this isn't worth the effort, since we are
	     *  going away?
	     */
	    if (!prev_funl)
		exit_funnel_nest ();
	}
    }
}


void gp_state_change (tid, prev_funl)
tid_t  tid;
int  *prev_funl;
{
    struct _rcmProc *pproc;
    struct _gscComProc *pcproc;
    struct _gscDev *pdev;
    int    old_int;

    /*
     *  Make sure we have a common structure.
     */
    old_int = i_disable(INTMAX);

    if (! apCom)
    {
	BUGLPR(dbg_rcmgp,BUGNFO,
	   ("gp_state_change: no apCom found\n"));

	i_enable (old_int);

	return;
    }

    /*
     *  Search for the common process structure using the tid.  We don't want
     *  to be fooled by nongraphical thread terminations in a graphical process.
     */
    for (pcproc = apCom->pProcList;
	 pcproc;
	 pcproc = pcproc->pNext)
    {
	if (pcproc->tid == tid)
	    break;
    }

    if (! pcproc)
    {
	BUGLPR(dbg_rcmgp,BUGNFO,
	   ("gp_state_change: no cproc found\n"));

	i_enable (old_int);

	return;
    }

    /*
     *  Beginning again at the common origin, search all the devices
     *  for at most 1 rcm process structure per device.  When such a
     *  structure is found, perform rcm_unmake_gp on it.
     *
     *  Since rcm_unmake_gp() can delete pdev, we need to be careful in
     *  our loop.
     */
    pdev = apCom->pDevList;
    while (pdev)
    {
	RCM_TRACE (0x920, getpid (), tid, pdev);

	/*
	 *  Search for any rcm process struc for our tid.
	 */
	for (pproc=pdev->devHead.pProc; pproc; pproc=pproc->procHead.pNext)
	{
	    if (pproc->procHead.tid == tid)
		break;
	}

	/*
	 *  If not found on this device, go to the next one.
	 */
	if (!pproc)
	    pdev = pdev->devHead.pNext;
	else			/* we DID find it */
	{
	    RCM_TRACE (0x921, getpid(), pproc, pdev);

	    i_enable (old_int);

	    BUGLPR(dbg_rcmgp,BUGNFO,
		("gp_state_change: unmake_gp, pdev 0x%x\n", pdev));

	    /* Notify driver of state change */
	    if (pdev->devHead.display->state_change)
	    {
		RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
		(void) (pdev->devHead.display->state_change) (pdev, pproc);
		RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
	    }

	    /* last unmake will update prev_funl on mp */
	    /* this may make pdev go away! */
	    rcm_unmake_gp (pdev, prev_funl);

	    old_int = i_disable (INTMAX);

	    /*
	     *  Since the pdev list can change, always start over.
	     */
	    if ( apCom ) 
		pdev = apCom->pDevList;
	    else
		break;
	}
    }

    i_enable (old_int);
}


void devno_state_change (tid)
tid_t  tid;
{
    int  i;

    /*
     *  Search for the graphics device structure.
     *
     *  No interrupt or thread protection is needed, since the flag/tid
     *  setting will be static for our process.  Clear in-use flag last.
     *
     *  Tid's are systemwide global, so no pid check need be made.
     */
    for (i=0; i<rcmdds->number_of_displays; i++)
    {
	if ((rcmdds->disp_info[i].flags & RCM_DEVNO_IN_USE) &&
	     rcmdds->disp_info[i].tid == tid                    )
	{
	    update_pm_data(rcmdds->disp_info[i].pid,REMOVE_DSP);
	    rcmdds->disp_info[i].pid = 0;
	    rcmdds->disp_info[i].tid = 0;
	    rcmdds->disp_info[i].handle = 0;
	    rcmdds->disp_info[i].flags &= ~RCM_DEVNO_IN_USE;

	    BUGLPR(dbg_rcmgp,BUGNFO,
		("devno_state_change: free display %s\n",
			rcmdds->disp_info[i].lname));
	}
    }
}

/* ============================================================= */
/* FUNCTION: gsc_set_gp_priority
*/
/* PURPOSE: sets the priority of a graphics process
*/
/* DESCRIPTION:
*/
/* INVOCATION:
    int gsc_set_gp_priority (pd, parg)
	struct phys_displays *pd;
	set_gp_priority 	*parg;
*/
/* CALLS:
*/
/* DATA:
	virtual terminal structure and RCM/DDH structure
*/
/* RETURNS:
	0 if successful
	ERRNO if unsuccessful
*/
/* OUTPUT:
	error code
*/
/* RELATED INFORMATION:
*/
/* NOTES:
	This function will not change the priority of a
	rendering context on a fault list. This is because
	the priority of an rcx is independent of that of
	the gp that owns it during the scheduling until
	the priority of the rcx has reached some minimum, at
	which time the rcx priority is set according to that
	of the owning gp.
*/

int gsc_set_gp_priority (pd, parg)
    struct phys_displays *pd;
    set_gp_priority	*parg;
{
    set_gp_priority	a;
    rcmProcPtr		pproc;
    gscDevPtr		pdev;

    gsctrace (SET_GP_PRIORITY, PTID_ENTRY);
    /* create device pointer */
    SET_PDEV(pd,pdev);

    /* check if caller is gp, return if not */
    FIND_GP(pdev,pproc);
    if (pproc == NULL) {
	BUGPR(("###### set_priority ERROR not a gp \n"));
	return (EINVAL);
    }
    BUGLPR(dbg_rcmgp,BUGACT,
	   ("====== set_priority... found proc=0x%x\n", pproc));

    /* copy the argument */
    if (copyin (parg, &a, sizeof (a))) {
	BUGPR(("###### set_priority ERROR copyin arg \n"));
	return (EFAULT);
    }

    BUGLPR(dbg_rcmgp,BUGNFO,
	   ("====== set_priority... priority=%d, pid=0x%x\n",
	    a.priority, a.pid));

    /* check parms */
    if (pproc->procHead.pid != a.pid) {
	BUGPR(("###### set_gp_priority ERROR, bad pid \n"));
	return (EINVAL);
    }
    if (a.priority < 0 || a.priority > MAX_PRIORITY) {
	BUGPR(("###### set_gp_priority ERROR, bad priority \n"));
	return (EINVAL);
    }
    BUGLPR(dbg_rcmgp,BUGACT,("====== pid and priority OK \n"));

    /* change the priority of the gp */
    pproc->procHead.priority = a.priority;
    gsctrace (SET_GP_PRIORITY, PTID_EXIT);

}

/* =============================================================

		INTERNAL FUNCTIONS

   ============================================================= */

/* free proc and other created rcx structures */

/*
 *  This is used AFTER the pproc structure is filled in and AFTER
 *  the DD level make_gp function is called.  Any rcx structures
 *  that have been filled in must be rightly linked.  That is all.
 */
void rcm_kill_proc (pproc)

    rcmProcPtr	pproc;

{
    gscDevPtr	pdev;
    rcxPtr	prcx, tprcx;

    pdev = pproc->procHead.pGSC;

    prcx = pproc->procHead.pRcx;
    while (prcx != NULL) {
	tprcx = prcx;
	prcx = prcx->pNext;
	/************** WARNING ***********************************/
	/* it is assumed that the device specific delete rcx	  */
	/* function does nothing other than delete data structures*/
	/* and so does not have to be guarded			  */
	/************** WARNING ***********************************/
        RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
	(pdev->devHead.display->delete_rcx) (pdev, tprcx);
        RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
	xmfree ((caddr_t) tprcx, pinned_heap);
    }

    /* call the device dependent unmake_gp function */
    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
    if (pdev->devHead.display->unmake_gp)
	    (pdev->devHead.display->unmake_gp) (pdev, pproc);
    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);

    /* deregister and delete bus protection pkt */
    detach_busprot (pproc);

    /* delete any seg/busprot setup for the rcm process struct */
    detach_seg_reg (pproc);

    xmfree ((caddr_t) pproc, pinned_heap);
}

/*
	com_init

    creates and initializes the RCM common structure,
    registers global fault handler and state change handler,
    initializes the Heavy Switch Controller

    assumes that com anchor locked
*/

static int com_init(pd)
struct phys_displays *pd;
{
    gscComPtr	pcom;
    int	gp_give_up_time_slice();
    int	gp_guard_domain();
    int	gp_unguard_domain();
    int	gp_block_gp();
    int	gp_unblock_gp();
    int	gp_make_cur_and_guard_dom();
    int	gp_put_on_fault_list();
    int i, rc;

    if (apCom != NULL)
	return  0;

    BUGLPR(dbg_rcmgp_trace,0,
	   ("**** com_init..TRACE.. trace_all=0x%x, anchor=0x%x\n",
	    &trace_all, &comAnc));

    BUGLPR(dbg_rcmgp,BUGNFO, ("\n==== Enter com_init pd 0x%x\n", pd));

    /* create the com structure */
    pcom = xmalloc (sizeof (struct _gscCom), 3, pinned_heap);
    if (pcom == NULL) {
	BUGPR(("###### com_init ERROR xmalloc com\n"));
	return (ENOMEM);
    }

    /* setup the HSC */
    if (rc = create_hsc()) {
	xmfree ((caddr_t) pcom, pinned_heap);
	BUGPR(("###### com_init ERROR no create hsc\n"));
	return (rc);
    }

    /* set up to register the graphics fault handler for this device */
    pcom->except.next = NULL;
    pcom->except.handler = (int (*) ())gp_fault;

    /*  Actual registration of the above handler comes at function end */

    /* set other initial values */
    pcom->pDevList = NULL;
    pcom->pProcList = NULL;
    pcom->pRcxParts = NULL;

    pcom->rcm_callback = xmalloc (sizeof (struct callbacks), 3, pinned_heap);
    if (pcom->rcm_callback == NULL)
    {
	xmfree ((caddr_t) pcom, pinned_heap);
	return (ENOMEM);
    }
    pcom->rcm_callback->give_up_timeslice = gp_give_up_time_slice;
    pcom->rcm_callback->guard_domain = gp_guard_domain;
    pcom->rcm_callback->unguard_domain = gp_unguard_domain;
    pcom->rcm_callback->block_graphics_process = gp_block_gp;
    pcom->rcm_callback->unblock_graphics_process = gp_unblock_gp;
    pcom->rcm_callback->make_cur_and_guard_dom = gp_make_cur_and_guard_dom;
    pcom->rcm_callback->put_on_fault_list = gp_put_on_fault_list;

    /* set up the common anchor structure to point to common */
    apCom = pcom;

    /* any errors below: don't forget to free pcom and pcom->rcm_callback */

    /* --------------
        font support: attach shared memory to font kernel process.
        X server will be the first gp calling gsc_make_gp.  Only then 
        we attach shared memory created by X to the font kernel process
        so that we can pin/unpin X fonts. 
    ---------------- */

    BUGLPR(dbg_rcmgp,BUGNFO,
		("com_init: call make_shm with vtmstruc ptr =%x\n",
		pd->visible_vt));

    i = make_shm(pd->visible_vt);

    BUGLPR(dbg_rcmgp,BUGNFO,
		("com_init: returned from make_shm rc=%d\n",i));

    if (i != 0)
    {
       BUGPR(("###### com_init ERROR make_shm failed, don't care\n"));
    }

    /* 
           When there are no more gp's, we want to detach the shared memory 
	   attached to font kernel process.

           To do so, we need access to the lft's common.  Since there is
           only one lft's common (we support multiple displays but there
           can be only 1 keyboard), we save the pointer to the lft's common 
           here in the RCM global data structure, apCom, and access it as we 
           need it.
    */

    apCom->lftCommonPtr = pd->lftanchor;

    /*
     *  Register the fault handler here at the end to avoid having to
     *  deregister it on the error paths above.
     */
    uexadd (&pcom->except);

    BUGLPR(dbg_rcmgp,BUGNFO,("==== Exit com_init, pcom=0x%x\n", pcom));

    /* return */
    return (0);
}

/*
	com_term

    frees the RCM common structure,
    unregisters global fault handler and state change handler,
    terminates the Heavy Switch Controller

    assumes that com anchor locked

    This is a NOP if any common process link is left.  This is to
    allow this function to be called in miscellaneous errors paths.
*/

static void com_term(pd)
struct phys_displays *pd;
{
    void	kill_hsc();
    struct _rcxp    *prcxpt, *prcxp;
    gscComPtr  ap;

    BUGLPR(dbg_rcmgp,BUGNFO, ("\n==== Enter com_term, apCom 0x%x\n", apCom));

    /* don't terminate if a common process struct is left */
    if (apCom->pProcList)
    {
	BUGLPR(dbg_rcmgp,BUGNFO, ("\n==== Exit com_term, proc left\n"));
	return;
    }

    /* ----------------
       font support:  need to detach shared memory from font kernel 
                      process.  Note we attached it to fkproc in 
                      gsc_make_gp
    ------------------- */

    BUGLPR(dbg_rcmgp,BUGNFO,("com_term: calling unmake_shm with vtmstruc ptr=%x\n",pd->visible_vt));

    (void) unmake_shm(pd->visible_vt);

    BUGLPR(dbg_rcmgp,BUGNFO,
			("com_term: returned from unmake_shm\n"));

    ap = apCom;
    apCom = NULL;

    /* unregister the exception handler for the RCM */
    uexdel (&ap->except);

    /* unpin and free rcx parts */
    /* no process sync required since apCom already NULL */
    for(prcxp = ap->pRcxParts; prcxp != NULL;) {
	prcxpt = prcxp;
	prcxp = prcxp->pNext;
	xmfree((caddr_t) prcxpt, pinned_heap);
    }

    /* terminate the HSC */
    kill_hsc ();

    /* free the structure */
    xmfree ((caddr_t) ap->rcm_callback, pinned_heap);
    xmfree ((caddr_t) ap, pinned_heap);

#ifdef  RCMDEBUG
    all_trace_reports (-1);
#endif

    BUGLPR(dbg_rcmgp,BUGNFO, ("\n==== Exit com_term\n"));
}

/* ============================================================= */
/* FUNCTION: dev_init
*/
/* PURPOSE: initializes the RCM structures for a virtual terminal
*/
/* DESCRIPTION:
	It creates and pins the gsc device structure.
        It creates and pins the window geometry hash table.
	It links the device structure to the virtual terminal
	structure. It sets up the exception handler for the
	device and the timers for the domains.
*/
/* INVOCATION:
	dev_init (pd, pcproc)
	struct phys_displays *pd;
	gscComProcPtr   pcproc;
*/
/* CALLS:
	NONE
*/
/* DATA:
	virtual terminal structure
*/
/* RETURNS:
	none-zero if problem with allocation or pinning
*/
/* OUTPUT:
	side effects only, creates and pins some data structures
*/
/* RELATED INFORMATION:
*/
/* NOTES:
	Assumes the RCM common structure is locked so the list
	of devices can be updated without danger.
*/

static int dev_init (pd, pcproc)

    struct phys_displays *pd;
    gscComProcPtr   pcproc;

{
    int 	i, old_int, rc, pdindex;
    gscDevPtr	pdev;
    int 	num_domains;
    rcm_wg_hash_table_t *wg_hash ;
    int         vttnop ();

    /* if already initialized, just return */
    if ((pdev = pd->pGSC) != NULL)
    {
	pdev->devHead.count++;

	BUGLPR(dbg_rcmgp,BUGNFO,
	   ("====== (re)dev_init... pd=0x%x pdev->devHead.count\n",
						pd, pdev->devHead.count));
	return  0;
    }

    /* for this to work right, we MUST be the graphics process leader */

    /* check if the device is set up reasonably */
    num_domains = pd->dwa_device ?
	pd->num_domains : 1;
    BUGLPR(dbg_rcmgp,BUGNFO,
      ("====== dev_init... pd=0x%x, pd->dwa_device 0x%x, pd->num_domains=%d\n",
	    pd, pd->dwa_device, pd->num_domains));
    if (num_domains <= 0 || num_domains > MAX_DOMAINS) {
	BUGPR(("###### dev_init ERROR bad domains in vt\n"));
	return (EINVAL);
    }

    /* sanity: search for our disp_info structure */
    for (pdindex=0; pdindex<rcmdds->number_of_displays; pdindex++)
    {
	if (rcmdds->disp_info[pdindex].pd == pd)
	    break;
    }
    assert (pdindex < rcmdds->number_of_displays);

    /* sanity: only the leader can pass the test */
    if (rcmdds->disp_info[pdindex].pid != getpid ())
	return (EINVAL);

    /* sanity: keep leader from releasing display */
    rcmdds->disp_info[pdindex].flags |= RCM_DEVNO_MAKE_GP;

    /* --------------------------------------------------------------------
       Need to send an ioctl to the lft notifying it that it needs to give
       up access to the selected display.
     * --------------------------------------------------------------------*/
    if ((rc = fp_ioctl(rcmdds->lft_info.fp,LFT_ACQ_DISP,&pd->devno,0)) != 0) {
	BUGLPR(dbg_rcmgp,BUGACT, 
		("dev_init: acquire display ioctl call to lft failed\n"));
        /* sanity: allow leader to release display */
        rcmdds->disp_info[pdindex].flags &= ~RCM_DEVNO_MAKE_GP;

	return(rc);
    }

    /* --------------------------------------------------------------------
       allocate and pin and initialize the gsc device structure
     * --------------------------------------------------------------------*/
 
    i = sizeof (struct _gscDevHead) +
	(num_domains * sizeof (struct _devDomain));
    BUGLPR(dbg_rcmgp,BUGACT, ("====== dev_init... dev size=%d\n", i));

    pdev = xmalloc (i, 3, pinned_heap);
    if (pdev == NULL) {
	/* release previously acquired lft display */
	fp_ioctl( rcmdds->lft_info.fp, LFT_REL_DISP, &pd->devno, 0);
	BUGPR(("###### dev_init ERROR xmalloc dev, size=%d\n", i));
        /* sanity: allow leader to release display */
        rcmdds->disp_info[pdindex].flags &= ~RCM_DEVNO_MAKE_GP;
	return (ENOMEM);
    }

    bzero (pdev, i);

    /* --------------------------------------------------------------------
        allocate and pin the window geometry hash table
     * --------------------------------------------------------------------*/

    wg_hash = xmalloc (sizeof(rcm_wg_hash_table_t), 3, pinned_heap);

    if (wg_hash == NULL)
    {
        BUGPR(("###### dev_init ERROR xmalloc wg_hash\n"));
        xmfree ((caddr_t) pdev, pinned_heap);
        /* release previously acquired lft display */
	fp_ioctl( rcmdds->lft_info.fp, LFT_REL_DISP, &pd->devno, 0);
        /* sanity: allow leader to release display */
        rcmdds->disp_info[pdindex].flags &= ~RCM_DEVNO_MAKE_GP;
        return (ENOMEM);
    }

    bzero (wg_hash, sizeof(rcm_wg_hash_table_t) );

    /* --------------------------------------------------------------------
        Initialize the graphic device structure
     * --------------------------------------------------------------------*/

    /* initialize the header */
    pdev->devHead.count = 1;
    pdev->devHead.rcmtrace = (rcmTracePtr) &trace_all;
    pdev->devHead.locknest = 0;
    pdev->devHead.locklist = EVENT_NULL;
    pdev->devHead.dma_sleep = EVENT_NULL;
    pdev->devHead.sync_sleep = EVENT_NULL;
    pdev->devHead.num_domains = num_domains;
    pdev->devHead.pCm = NULL;

    pdev->devHead.wg_hash_table  = wg_hash ;
    pdev->devHead.window_count   = 0;

    /* set up redundant pointers in pdev */
    pdev->devHead.display = pd;
    pdev->devHead.vttld = pd->visible_vt->vttld;

    pdev->devHead.leader_pid = getpid ();
    pdev->devHead.leader_tid = thread_self ();

    /* initialize the domain array */
    for (i=0; i < pdev->devHead.num_domains; i++)
    {
	pdev->domain[i].rlock = LOCK_AVAIL;
	pdev->domain[i].guardlist = EVENT_NULL;
	pdev->domain[i].pDev = pdev;
	pdev->domain[i].domain = i;

	/* set up domain timer, with gp_dispatch servicing timer interrupts */
	pdev->domain[i].pDevTimer = talloc();
	if (pdev->domain[i].pDevTimer == NULL)
	{
	    for (i--; i>=0; i--)
		tfree (pdev->domain[i].pDevTimer);

	    xmfree ((caddr_t) wg_hash, pinned_heap);
	    xmfree ((caddr_t) pdev, pinned_heap);
	    /* release previously acquired lft display */
	    fp_ioctl( rcmdds->lft_info.fp, LFT_REL_DISP, &pd->devno, 0);
            /* sanity: allow leader to release display */
            rcmdds->disp_info[pdindex].flags &= ~RCM_DEVNO_MAKE_GP;

	    return  ENOMEM;
	}

	pdev->domain[i].pDevTimer->func = (void (*) ())gp_dispatch;
	pdev->domain[i].pDevTimer->t_func_addr = (caddr_t) &pdev->domain[i];
	pdev->domain[i].pDevTimer->ipri = INTMAX;
		/* rest of timer information must be set up at the time
		   a rcx is made current (and timer started) */
    }

    /*
     *  Link the gsc device structure to the vt.  THIS IS THE CRITICAL FIELD 
     *  WHICH INDICATES EXISTENCE OF THE DEVICE STRUCTURE!
     */
    pd->pGSC = (struct _gscDev *) pdev;

    /*
     *  Possibly initialize pVT calldown function pointer for vttddf_fast.
     *
     *  Nothing will be done here if the driver configuration process
     *  initialized the pointer in the phys_display structure to a non-NULL
     *  value.
     *
     *  If the pointer is still NULL at this point (we are initializing a
     *  graphical virtual terminal on that physical device), then we will
     *  initialize the pointer to vttnop to avoid having NULL checks each
     *  time vttddf_fast is called.  The DDdev_init code is called after this,
     *  giving the driver a chance to set the vttddf_fast function pointer to
     *  something else, if desired.
     *
     *  The vttddf_fast pointer is used only by the RCM.  All instances of this
     *  function are serialized under the common lock.  Therefore, there can
     *  no race condition with other users of the vttddf_fast pointer on this
     *  device.
     */
    if (pd->vttddf_fast == NULL)
	pd->vttddf_fast = vttnop;

    /* initialize the only the bus memory access keys and I/O ranges in the 
       domain array.  This calls DDdev_init!
    */
    rc = domain_acc_authority_init(pcproc,pd);

    /* trace the return value.  That's all we want to know */
    RCM_TRACE(0x603,getpid(),pcproc,rc)

    if (rc != 0)    /* if there is any error */
    {
	for (i=0; i < pdev->devHead.num_domains; i++)
	    tfree (pdev->domain[i].pDevTimer);

	xmfree((caddr_t) pdev,pinned_heap); /* free space malloced for device */
	xmfree((caddr_t) wg_hash,pinned_heap); /* free space for hash table */
	pd->pGSC = NULL;
        /* release previously acquired lft display */
	fp_ioctl( rcmdds->lft_info.fp, LFT_REL_DISP, &pd->devno, 0);
        /* sanity: allow leader to release display */
        rcmdds->disp_info[pdindex].flags &= ~RCM_DEVNO_MAKE_GP;
	return (rc);
    }

    /* link the device structure to common */
    old_int = i_disable (INTMAX);
    pdev->devHead.pNext = apCom->pDevList;
    apCom->pDevList = pdev;
    pdev->devHead.pCom = apCom;
    i_enable (old_int);

    BUGLPR(dbg_rcmgp,BUGNFO, ("==== Exit dev_init... pdev=0x%x\n\n", pdev));

    return (0);
}

/* ============================================================= */
/* FUNCTION: dev_term
*/
/* PURPOSE: terminates the RCM structures for a virtual terminal
*/
/* DESCRIPTION:
	It frees and unpins the gsc device structure.
        It frees and unpins the window geometry hash table.
	It unlinks the device structure from the virtual terminal
	structure. It unregisters  the exception handler for the
	device and the timers for the domains.
*/
/* INVOCATION:
void	dev_term (pd, pcproc)
	struct phys_displays *pd;
	gscComProcPtr   pcproc;
*/
/* CALLS:
	NONE
*/
/* DATA:
	virtual terminal structure
*/
/* RETURNS:
*/
/* OUTPUT:
*/
/* RELATED INFORMATION:
*/
/* NOTES:
	Assumes the RCM common structure is locked so that the list
	of devices does not get corroded.

	All the process structures are gone or would not
	be calling this function. Assume all window geometries
	are gone too, since they get created by processes and
	should die with the gp that created them.
*/

static void dev_term (pd, pcproc)
    struct phys_displays *pd;
    gscComProcPtr   pcproc;

{
    int 	i, old_int, pdindex;
    gscDevPtr	pdev, pdevt;
    rcmCmPtr	pcm, pcm_n;

    /* set the device pointer for further operations */
    SET_PDEV(pd,pdev);

    RCM_ASSERT (pdev != NULL, 0, 0, 0, 0, 0);

    /*
     *  If we are the graphics process leader, then do the lft stuff, even
     *  if we are not the last one to perform dev_term on the head.
     *
     *  It would be more symmetrical to do this at the end of dev_term,
     *  but, dev_term can only call DDdev_term in the drivers.  Those
     *  are now stubs, and probably will never interfere with ownership
     *  of the head going back to the lft at this time.
     */
    if (pdev->devHead.leader_pid == getpid ())
    {
	struct ps_s pr_sp;  /* presentation space           */
	/* need to reinitialize display since gp no longer owns it */
	pr_sp.ps_w      = -1;
	pr_sp.ps_h      = -1;

	if ( (*pd->vttinit)(pd->visible_vt, NULL, &pr_sp) != 0)
            BUGLPR(dbg_rcmgp,BUGNFO,
                  ("dev_term: vttinit failed to reinitialize the display\n"));

	if ( (*pd->vttact)(pd->visible_vt) != 0)
	    BUGLPR(dbg_rcmgp,BUGNFO,
		  ("dev_term: vttact failed to reinitialize the display\n"));

	/* need to send an ioctl call to the lft to release the display */
	if( fp_ioctl( rcmdds->lft_info.fp, LFT_REL_DISP, &pd->devno, 0) )
    	    BUGLPR(dbg_rcmgp,BUGNFO,
		  ("dev_term: ioctl to lft failed to release the display\n"));

	/* sanity: find our display info struct */
	for (pdindex=0; pdindex<rcmdds->number_of_displays; pdindex++)
	{
	    if (rcmdds->disp_info[pdindex].pd == pd)
		break;
	}
	assert (pdindex<rcmdds->number_of_displays);

	/* sanity: allow the leader to release the display */
	rcmdds->disp_info[pdindex].flags &= ~RCM_DEVNO_MAKE_GP;
    }

    /*  don't terminate if rcm process structure is left */
    if (--pdev->devHead.count > 0)
	return;

    BUGLPR(dbg_rcmgp,BUGNFO,
	   ("\n==== Enter dev_term, address=0x%x\n", dev_term));

    /*
     *  THIS IS THE CRITICAL ITEM WHICH INDICATES NON-EXISTENCE OF THE
     *  DEVICE STRUCTURE.
     */
    old_int = i_disable (INTMAX);

    pd->pGSC = NULL;

    /* unlink the device from the RCM common structure */
    if ((pdevt = apCom->pDevList) == pdev) { /* first */
	apCom->pDevList = pdev->devHead.pNext;
    } else { /* not first, search rest */
	for (; pdevt->devHead.pNext != NULL; pdevt = pdevt->devHead.pNext)
	    if (pdevt->devHead.pNext == pdev) break;
	RCM_ASSERT(pdevt!=NULL, 0, 0, 0, 0, 0); /* should be in list */
	pdevt->devHead.pNext = pdevt->devHead.pNext->devHead.pNext;
    }

    /* for all domains */
    for (i=0; i < pdev->devHead.num_domains; i++) {
	/* stop timer and free the timer structure */
	if(pdev->domain[i].flags & DOMAIN_TIMER_ON)
		tstop (pdev->domain[i].pDevTimer);
	tfree (pdev->domain[i].pDevTimer);
    }

    i_enable (old_int);

#ifdef WG_DEBUG
    /*---------------------------------------------------------------
      Conditionally Check for leftover window geometries 
      and free the window geometry hash table.  
    ---------------------------------------------------------------*/
    {
        int 	hash_index;

        for ( hash_index = 0; hash_index < RCM_WG_HASH_SIZE; hash_index++)
        {
	    if ( pdev->devHead.wg_hash_table->entry[hash_index].pWG )
	    {
    		BUGLPR(dbg_rcmgp,BUGNFO,
		  ("\n Window Geometry leftover in WG hash table ! ! ! \n"));
		brkpoint(0xD06D00D0);
	    }
        }
    }
#endif

    xmfree ((caddr_t) pdev->devHead.wg_hash_table, pinned_heap);

    /* free all color maps (that are left) */
    pcm = pdev->devHead.pCm;
    while (pcm)
    {
	pcm_n = pcm->nxtCm;
	xmfree ((caddr_t) pcm, pinned_heap);
	pcm = pcm_n;
    }

    /*
     *  Call the DD level so it can take down device related info. Currently, 
     *  it is a no-op; though it might some day mean to clear out TCW's.
     *
     */
    if (pdev->devHead.display->dev_term)
	(pdev->devHead.display->dev_term) (pdev);

    /*
     *  Release all bus memory keys.
     */
    put_busmem_keys (pcproc, pdev);

    /* free the device structure */
    xmfree ((caddr_t) pdev, pinned_heap);

    BUGLPR(dbg_rcmgp,BUGNFO,("\n==== Exit dev_term\n"));
}

/*
    cproc_init

    creates and initializes a common process structure,
    attaches the segment used to address devices

    Assumes the RCM common structure is locked so can change
    common process list without problems.
*/

int cproc_init (ppcproc, prev_funl)

    gscComProcPtr	*ppcproc;
    int			*prev_funl;

{
    gscComProcPtr	pcproc;
    ulong		old_int;

    pcproc = *ppcproc;

    if (pcproc != NULL)
    {
	if (pcproc->tid == thread_self ())
	{
	    pcproc->count++;
	    return  0;
	}
	else			/* wrong thread */
	    return  EBUSY;
    }

    BUGLPR(dbg_rcmgp,BUGNFO, ("\n==== Enter cproc_init\n"));

    /* create the com proc structure */
    pcproc = xmalloc (sizeof (struct _gscComProc), 3, pinned_heap);
    if (pcproc == NULL) {
	BUGPR(("###### cproc_init ERROR xmalloc comproc\n"));
	return (ENOMEM);
    }

    *ppcproc = pcproc;

    pcproc->count = 1;
    pcproc->flags = 0;

    /* set kernel process id */
    pcproc->pid = getpid();
    pcproc->tid = thread_self();

    /* initialize pointer to usr buffer header array */
    pcproc->pusrbufhdr = NULL;

    /* clear segment register database */
    pcproc->pSegreg = NULL;

    /*
     *  Initialize ptr to bus memory key database.
     *  This will be created and released by the busmem utilities,
     *  as called from the dev_init and dev_term functions.
     */
    pcproc->pBusKey = NULL;

    /*
     *  Set COMPROC_FUNNELLED if first make-gp is from funnelled process.
     *  Always make sure that the flag at *prev_funl is nonzero to retain
     *  funnelling.
     */
    if (*prev_funl)				/* if previously funnelled */
	pcproc->flags |= COMPROC_FUNNELLED;
    else
	*prev_funl = 1;

    /*
     *  Make change unitary for list readers (who don't lock).
     *  We are only called while we have the common lock.
     */
    old_int = i_disable (INTMAX);

    /* link  common process structure to common structure */
    pcproc->pNext = apCom->pProcList;
    apCom->pProcList = pcproc;

    i_enable (old_int);

    BUGLPR(dbg_rcmgp,BUGNFO,
	   ("==== Exit cproc_init, pcproc=0x%x\n", pcproc));

#ifdef  RCMDEBUG
	/*
	 *  Print each time a new process starts.
	 */
	printf ("RCMDEBUG: &trace_all = 0x%x\n", &trace_all);
#endif

    /* return */
    return (0);
}

/*
    cproc_term

    detaches the segment used to address devices,
    frees a common process structure

    Assumes the RCM common structure is locked so can change
    common process list without problems.
*/

static void cproc_term (pcproc, prev_funl)

    gscComProcPtr	pcproc;
    int			*prev_funl;

{
    gscComProcPtr	pcproct;
    ulong		old_int;
    int rc;

    BUGLPR(dbg_rcmgp,BUGNFO, ("\n==== Enter cproc_term\n"));

    RCM_ASSERT (pcproc->count > 0, 0, 0, 0, 0, 0);

    pcproc->count--;
    if (pcproc->count != 0)
	return;

    /* restore indicator of previous funnelling status */
    if (!(pcproc->flags & COMPROC_FUNNELLED))
	*prev_funl = 0;

    /* disable interrupts to change list */
    old_int = i_disable (INTMAX);

    /* unlink the common process structure from the RCM common structure */
    if ((pcproct = apCom->pProcList) == pcproc) { /* first */
	apCom->pProcList = pcproc->pNext;
    } else { /* not first, search rest */
	for (; pcproct->pNext != NULL; pcproct = pcproct->pNext)
	    if (pcproct->pNext == pcproc) break;
	RCM_ASSERT(pcproct!=NULL, 0, 0, 0, 0, 0); /* should be in list */
	pcproct->pNext = pcproct->pNext->pNext;
    }

    /* enable interrupts */
    i_enable (old_int);

    /*
     *  Release all user buffer attachments in the user buffer pool
     *  for this process on all heads.
     *
     *  All user buffer attachments FOR ALL HEADS will be dropped,
     *  since all heads have unmade.
     */
    if (pcproc->pusrbufhdr != NULL)
    {
	rcm_usr_buffer_detach_all (pcproc);

	xmfree ((caddr_t) pcproc->pusrbufhdr, pinned_heap);
    }

    assert (pcproc->pSegreg == NULL);

    /* free the com proc structure */
    xmfree ((caddr_t) pcproc, pinned_heap);

#ifdef RCMDEBUG
    all_trace_reports (getpid ());
#endif

    BUGLPR(dbg_rcmgp,BUGNFO, ("==== Exit cproc_term"));
}

int vttnop ()
{
    return  0;
}

/* ============================================================= */
/* FUNCTION: fix_mstsave
*/
/* PURPOSE: sets mstsave area of ublock to give (or take away) access to
	    a device domain
*/
/* DESCRIPTION:
	It adds/subtracts the authority mask to/from the mstsave
	and an I/O range to/from the mstsave.

*/
/* INVOCATION:
   void fix_mstsave (pproc, pdom, flags)
	rcmProcPtr	pproc;
	devDomainPtr	pdom;
	int		flags;
*/
/* CALLS:
	NONE
*/
/* DATA:
	ublock - mstsave area
*/
/* RETURNS: none
*/
/* OUTPUT:
	ublock - mstsave area
*/
/* RELATED INFORMATION:
*/
/* NOTES:

	This doesn't handle bus I/O any more.  The make-gp code
	will disallow any device from being a switching adapter
	if it has bus I/O.

	It may be necessary to disable around this function.
*/

/**//* check disable requirement, since kernel calls do the work */
void fix_mstsave (pproc, pdom, flags)

    rcmProcPtr	    pproc;
    devDomainPtr    pdom;
    int 	    flags;

{
    struct rcmbusprot	*rcmbusprot;

    RCM_ASSERT(pproc, 0, 0, 0, 0, 0);

    RCM_TRACE(0x950,pproc,pdom->auth,flags);

    rcmbusprot = pproc->procHead.pRcmbusprot;

    while (rcmbusprot != NULL)
    {
	if (rcmbusprot->domain == pdom->domain)
	    break;

	rcmbusprot = rcmbusprot->next;
    }
    assert (rcmbusprot != NULL);

    if (flags & FIX_MSTSAVE_ADD)		/* adding authority */
	grant_display_owner (&rcmbusprot->busProt);
    else					/* subtracting authority */
	revoke_display_owner (&rcmbusprot->busProt);

    /* old bus I/O add/revoke (which didn't work right) was not used and
	has been removed.  */

    BUGLPR(dbg_rcmgp_int,BUGNFO,("\n==== Exit fix_mstsave\n"));
}
