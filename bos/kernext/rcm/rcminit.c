static char sccsid[] = "@(#)55	1.11  src/bos/kernext/rcm/rcminit.c, rcm, bos41J, 9520A_all 5/3/95 11:44:22";

/*
 * COMPONENT_NAME: (rcm) Rendering Context Manager Initialization
 *
 * FUNCTIONS:
 *
 * This main line file contains the graphics device driver interface
 * routines:
 *   rcmconfig			- RCM configuration routine
 *   rcmopen			- RCM open routine
 *   rcmclose			- RCM close routine
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989-1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <lft.h>                    /* includes for all lft related data */
#include <sys/device.h>
#include <sys/sysmacros.h>		/* for MAJOR/MINOR */
#include <sys/malloc.h> 		/* memory allocation routines */
#include <sys/syspest.h>
#include <sys/uio.h>
#include "rcmras.h"			/* error logging defines */
#include "rcm_mac.h"
#include <rcmdds.h>			/* RCM dds structure */
#include "xmalloc_trace.h"
#include "rcm_pm.h"
#include <sys/pm.h>

extern rcm_dds *rcmdds;

static int rcm_init = 0;

int rcmopen();
int rcmclose();
int aixgsc();
extern void rcm_state_change ();

extern void rcm_pm_data_init();
extern int update_pm_data();
extern int cleanup_pm_data();

BUGVDEF (dbg_rcminit, 99);

int
rcmconfig( devno, command, uio_ptr )
dev_t   devno;                          /* Major and Minor device number */
int     command;                        /* Command to perform            */
struct  uio     *uio_ptr;               /* UIO pointer with DDS data area */
{
	extern  int     rcmioctl();
	extern  int     nodev();

        struct  devsw   rcm_devsw;      /* Buffer to hold devsw entry   */
	int dds_sz;                 /* Computed size of dds struc   */
	struct iovec *iov;
	int       i, lock_stat, rc = 0;

	RCM_LOCK (&comAnc.rlock, lock_stat);

	switch (command)
	{
	    case CFG_INIT: /* Initialize RCM Device */

		if (rcm_init)
		{
			rc = EBUSY;
			break;		/* exit switch */
		}

	        /*
	          Allocate and initialize (to zero)  the lft_dds struct.
	          We would normally use uiomove, but in this case the size of
	          the dds is dependent upon the number of displays. This is set
	          up in the rcm config method (cfgrcm).  So instead of the 
		  uiomove we do a copyin.  The base address is contained in the
		  uio struct and so is the length.  This should not be a 
	  	  problem as uiomove ends up doing a vmcopyin which is the same
		  function that copyin calls.
	        */

	        iov = uio_ptr->uio_iov;     /* Get the iov structure        */
	        dds_sz = iov->iov_len;      /* Length of the dds structure  */
	
		/*  Sanity check -- must allow for at least one display */
		BUGLPR (dbg_rcminit, BUGNFO,
			("rcmconfig: CFG_INIT: dds_sz %d\n", dds_sz));

	        if (dds_sz < sizeof (rcm_dds))
		{
		    rc = EINVAL;
		    break;		/* exit switch */
		}

		if ((rcmdds = (rcm_dds *) xmalloc (dds_sz, 0, pinned_heap))
									== NULL)
		{
		    rc = ENOMEM;
		    break;		/* exit switch */
		}

	        rc = copyin (iov->iov_base, rcmdds, dds_sz);
	        if (rc)                  /* Error               */
	        {
	            xmfree ((caddr_t) rcmdds, pinned_heap);
		    break;		/* exit switch */
	        }

		/* Clear out new stuff cfg method didn't know about */
		BUGLPR (dbg_rcminit, BUGNFO,
			("rcmconfig: CFG_INIT: number_of_displays %d\n",
				rcmdds->number_of_displays));

		for (i=0; i<rcmdds->number_of_displays; i++)
		{
		    unsigned int     status;
		    struct phys_displays  *pd;

		    rcmdds->disp_info[i].flags  = 0;
		    rcmdds->disp_info[i].pid    = 0;
		    rcmdds->disp_info[i].tid    = 0;
		    rcmdds->disp_info[i].handle = 0;

		    /*
		     *  Get phys_displays pointer into the graphics device
		     *  devno table.
		     */
		    if (rc = devswqry (rcmdds->disp_info[i].devno,
				       &status,
				       (caddr_t *) &pd)              )
		    {
			BUGLPR(dbg_rcminit, BUGNFO,
				("rcminit: devno 0x%x is incorrect, rc %d\n",
						devno, rc));

			break;			/* exit loop */
		    }

		    /*
		     *  Verify that the devno is in the proper state.
		     */
#define RCM_DSW_OK  (DSW_OPENED | DSW_DEFINED)

		    if ( ! ((status & RCM_DSW_OK) == RCM_DSW_OK)  )
		    {
			BUGLPR(dbg_rcminit, BUGNFO,
				("rcminit: devno 0x%x is not available\n",
						devno));

			rcmdds->disp_info[i].flags |= RCM_DRIVER_NOTOK;
		    }

		    /*
		     *  Find the correct phys_disp structure
		     */
		    while (pd != NULL                              &&
			   pd->devno != rcmdds->disp_info[i].devno    )
		    {
			pd = pd->next;
		    }

		    if (pd == NULL)
		    {
			BUGLPR(dbg_rcminit, BUGNFO,
				("rcminit: no pd found for devno 0x%x\n",
						rcmdds->disp_info[i].devno));

			rc = EINVAL;
			break;			/* exit loop */
		    }

		    rcmdds->disp_info[i].pd = pd;
		}

		/* in case we broke the loop on an error */
		if (i < rcmdds->number_of_displays)
		{
	            xmfree ((caddr_t) rcmdds, pinned_heap);
		    break;		/* exit switch */
		}

		/* Add entry into the device switch table */
		rcm_devsw.d_open = rcmopen;     /* Open entry point     */
		rcm_devsw.d_close = rcmclose;   /* Close entry point    */
		rcm_devsw.d_ioctl = rcmioctl;   /* Ioctl entry point    */
		rcm_devsw.d_config = rcmconfig; /* Config entry point   */
                rcm_devsw.d_read = nodev;	/* entry point not supported */
                rcm_devsw.d_write = nodev;	/* entry point not supported */
                rcm_devsw.d_strategy = nodev;	/* entry point not supported */
                rcm_devsw.d_ttys = NULL;	/* tty struct not supported */
                rcm_devsw.d_select = nodev;	/* entry point not supported */
                rcm_devsw.d_print = nodev;	/* entry point not supported */
                rcm_devsw.d_dump = nodev;	/* entry point not supported */
                rcm_devsw.d_mpx = nodev;	/* entry point not supported */
                rcm_devsw.d_revoke = nodev;	/* entry point not supported */
                rcm_devsw.d_dsdptr = (caddr_t) rcmdds;	/* ptr to the rcm dds */

		if ( rc = devswadd( devno, &rcm_devsw ))
		{
			BUGLPR(dbg_rcminit, 0,
				("rcmconfig: devswadd failed, rc %d\n", rc));
			xmfree ((caddr_t) rcmdds, pinned_heap);
			break;		/* exit switch */
		}

		/* Pin modules that will be called on the interrupt level */
		if ( rc = pincode((void (*)()) aixgsc))
		{
			assert (devswdel ( devno ));
			xmfree ((caddr_t) rcmdds, pinned_heap);
			break;		/* exit switch */
		}

		/* to register the state change handler to catch exiting gps */
		comAnc.state_chng.next = NULL;
		comAnc.state_chng.handler = rcm_state_change;

		prochadd (&comAnc.state_chng);

		rcm_pm_data_init(devno);

		pm_register_handle(&pmdata,PM_REGISTER);    /* make RCM a PM aware pseudo-device */

		/* the rcm dd is now usable. */
		rcm_init = 1;

		break;		/* exit switch */

	    case CFG_TERM: /* Terminate RCM Device */

		if ( (rcmdds->open_count != 0) ||  (pm_event) )
		{
		    rc = EBUSY;
		    break;		/* exit switch */
		}

		/* unregister handler with PM core */
		pm_register_handle(&pmdata, PM_UNREGISTER);  

		/*
		 *  Since the rcm dd is unusable if ANY of the following
		 *  kernel calls work (and therefore take away resources),
		 *  don't let it be opened again without re-init.
		 */
		rcm_init = 0;

		/* unregister the state change handler */
		prochdel (&comAnc.state_chng);

               /* Unpin modules that will be called on the interrupt level */
                assert (unpincode((void (*)()) aixgsc) == 0);

		assert (devswdel( devno ) == 0 );
	 
		xmfree((caddr_t) rcmdds, pinned_heap);

#ifdef  RCMDEBUG
		all_trace_reports (-1);
#endif

                break;		/* exit switch */

	    default:
		rc = EINVAL;

		break;		/* exit switch */
	}

	BUGLPR (dbg_rcminit, BUGNFO, ("rcminit: exit, rc %d\n", rc));

	RCM_UNLOCK (&comAnc.rlock, lock_stat);

#ifdef  RCM_SIGSYS
	if (rc)
		pidsig (getpid (), SIGSYS);
#endif
        return(rc);
}

int
rcmopen(devno, rwflag, channel, ext )
dev_t   devno;                          /* Device Number                */
int     rwflag;                         /* Open for read/write or both  */
int     channel;                        /* channel to use               */
int     ext;                            /* Extension                    */

{
	int	rc, lock_stat;

	RCM_LOCK (&comAnc.rlock, lock_stat);

	if ( ! rcm_init )
	{
		RCM_UNLOCK (&comAnc.rlock, lock_stat);

#ifdef  RCM_SIGSYS
		if (rc)
			pidsig (getpid (), SIGSYS);
#endif
		return(ENXIO);
	}

	BUGLPR (dbg_rcminit, BUGNFO, ("rcmopen: enter\n"));

	/* open the lft if not already open */
	if ( rcmdds->lft_info.fp == NULL )
	{
		BUGLPR (dbg_rcminit, BUGNFO, ("rcmopen: open lft\n"));

		if ( (rc = fp_opendev( rcmdds->lft_info.devno, DWRITE, NULL, 0, 
				&rcmdds->lft_info.fp )) != 0 )
		{
			BUGLPR (dbg_rcminit, BUGNFO,
				("rcmopen: open lft failed, rc %d\n", rc));

			RCM_UNLOCK (&comAnc.rlock, lock_stat);

#ifdef  RCM_SIGSYS
			pidsig (getpid (), SIGSYS);
#endif
			return(rc);
		}
	}


	/*
	 *  Every instance of a graphics process leader (X) (currently only
	 *  one instance allowed) will call this open entry.
	 */
	rcmdds->open_count = 1;		/* don't bother counting */

	RCM_UNLOCK (&comAnc.rlock, lock_stat);

        return(0);
}

rcmclose( devno, channel, ext )
dev_t   devno;                          /* Device Number                */
off_t   channel;                        /* VT id (mpx/channel number)   */
int     ext;                            /* Unused - (Extension)         */
{
	int rc, lock_stat, i, inuse;

	RCM_LOCK (&comAnc.rlock, lock_stat);

	BUGLPR (dbg_rcminit, BUGNFO, ("rcmclose: entered\n"));

	/*
	 *  Every instance of graphics process leader performing a close
	 *  on this device will NOT result in a call to this function
	 *  from the kernel interface until the LAST nested close occurs.
	 *
	 *  If the last open state of the rcm is about to be closed,
	 *  and if the LFT is still open to the RCM, close the LFT.
	 */
	if (rcmdds->open_count == 0)
	{
		BUGLPR (dbg_rcminit, BUGNFO, ("rcmclose: not open\n"));

		RCM_UNLOCK (&comAnc.rlock, lock_stat);

#ifdef  RCM_SIGSYS
		pidsig (getpid (), SIGSYS);
#endif
		return (ENXIO);
	}

	/*
	 *  Release devices which are still held, but not in graphics state.
	 */
	inuse = 0;			/* init flag: none in graphics state */
	for (i=0; i<rcmdds->number_of_displays; i++)
	{

		if (rcmdds->disp_info[i].flags & RCM_DEVNO_IN_USE)
		{

			/*
			 *  At this point we could try to perform an automatic
			 *  unmake-gp, but we don't.  User's already know to
			 *  do this in in-line code.  So, if this condition
			 *  occurs we just return an error (after processing
			 *  the whole loop).  We also don't treat any device
			 *  not owned by the current process.  (This is not
			 *  supposed to happen).
			 *
			 *  Tid's are systemwide global, so no pid check need
			 *  be made.
			 */
			if (rcmdds->disp_info[i].flags & RCM_DEVNO_MAKE_GP ||
			    rcmdds->disp_info[i].tid  != thread_self ()       )
			{
			    BUGLPR (dbg_rcminit, BUGNFO,
				("rcmclose: %s still in use\n",
					rcmdds->disp_info[i].lname));
			    inuse++;
			    continue;
			}

			/*
			 *  Since the graphics mode is not in effect, a close
			 *  should imply a release of the device.  Now, since
			 *  the rcm is a regular AIX (UNIX) device driver, the
			 *  close call actually won't come here until the last
			 *  process who has it open closes it.  But, since
			 *  we believe that at this stage of the game only one
			 *  process at a time uses device complex, the close
			 *  will probably always come here.
			 *
			 *  We just release any device-in-use state for this
			 *  process.
			 */

			BUGLPR (dbg_rcminit, BUGNFO,
				("rcmclose: clearing inuse state for %s\n",
					rcmdds->disp_info[i].lname));

			update_pm_data(rcmdds->disp_info[i].pid,REMOVE_DSP);  

			/* clear in-use flag first */
			rcmdds->disp_info[i].flags &= ~RCM_DEVNO_IN_USE;
			rcmdds->disp_info[i].pid    = 0;
			rcmdds->disp_info[i].tid    = 0;
			rcmdds->disp_info[i].handle = 0;

		}
	}

	if (inuse)
	{
		RCM_UNLOCK (&comAnc.rlock, lock_stat);

		BUGLPR (dbg_rcminit, BUGNFO,
				("rcmclose: devices still in use\n"));
#ifdef  RCM_SIGSYS
		pidsig (getpid (), SIGSYS);
#endif
		return  EBUSY;
	}

	/* process terminates so reset RCM PM data  */
	cleanup_pm_data(getpid());  

	/*
	 *  Close the lft.
	 */
	assert (rcmdds->lft_info.fp != NULL);

	BUGLPR (dbg_rcminit, BUGNFO, ("rcmclose: close lft\n"));

	if (( rc = fp_close( rcmdds->lft_info.fp )) != 0 )
	{
		BUGLPR (dbg_rcminit, BUGNFO,
				("rcmclose: close lft failed, rc %d\n", rc));

		RCM_UNLOCK (&comAnc.rlock, lock_stat);

#ifdef  RCM_SIGSYS
		pidsig (getpid (), SIGSYS);
#endif
		return(rc);
	}

	rcmdds->lft_info.fp = NULL;

	rcmdds->open_count = 0;		/* Called by kernel on last close */

	RCM_UNLOCK (&comAnc.rlock, lock_stat);

	return(0);
}
